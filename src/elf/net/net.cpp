/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/lock.h>
#include <elf/net/net.h>
#include <elf/net/message.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <elf/time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <string>

namespace elf {
static const int LINGER_ONOFF = 0;
static const int LINGER_TIME = 5;
static const int CONTEXT_CLOSE_TIME = 300;
static const int SIZE_INT = sizeof(int(0));
static const int SIZE_INTX2 = sizeof(int(0)) * 2;
static const int MESSAGE_LITE_SIZE = 10;
static const int CHUNK_MIN_SIZE = 4096;
static const int CHUNK_MAX_SIZE = 65536;
static const size_t CHUNK_MAX_NUM = 1024;
static const int MESSAGE_MAX_NAME_LENGTH = 100;
static const int MESSAGE_MAX_VALID_SIZE = CHUNK_MAX_NUM * CHUNK_MAX_SIZE;
static const int MESSAGE_MAX_PENDING_SIZE = MESSAGE_MAX_VALID_SIZE * 2;
static const int BACKLOG = 128;
static const int ENCRYPT_FLAG = 0x40000000;

struct blob_t;
struct chunk_t;
struct context_t;
struct peer_t;
struct recv_message_t;
struct stat_msg_t;

typedef std::list<chunk_t *> chunk_queue;
typedef std::map<std::string, stat_msg_t *> msg_map;
typedef std::queue<context_t *> free_context_queue;
typedef std::deque<recv_message_t *> recv_message_queue;
typedef xqueue<recv_message_t *> recv_message_xqueue;

struct stat_msg_t {
    int msg_num; // number of recv given msg
    int msg_size; // size of recv given msg

    stat_msg_t() :
        msg_num(0), 
        msg_size(0)
    {
    }
};

struct stat_t {
    int msg_num; // number of recv msg
    int msg_size; // size of recv msg
    msg_map req_msgs; // request message map
    msg_map res_msgs; // response message map

    stat_t() :
        msg_num(0), 
        msg_size(0)
    {
    }
};

struct blob_t {
    chunk_queue chunks;
    int msg_size; // current recv msg size(for splicing)
    int total_size; // total send/recv msg size
    int pending_size; // pending send/recv msg size
};

struct chunk_t {
    int wr_offset;
    int rd_offset;
    int size;
    char data[0];
};

struct peer_t {
    int idx;
    int sock;
    oid_t id;
    char ip[INET_ADDRSTRLEN];
    char ipv6[INET6_ADDRSTRLEN];
    int port;
    char info[64];
};

struct context_t {
    peer_t peer;
    mutex_t lock;
    blob_t *recv_data;
    blob_t *send_data;
    cipher_t *encipher;
    cipher_t *decipher;
    epoll_event evt;
    int start_time;
    int close_time;
    int last_time;
    int error_times;
    bool internal;
};

typedef std::map<oid_t, context_t *> context_map;
typedef std::set<oid_t> context_set;

static thread_t s_tid; // io thread
static thread_t s_cid; // context thread
static spin_t s_context_lock;
static spin_t s_pre_context_lock;
static stat_t s_stat;
static int s_epoll;
static int s_sock;
static int s_sock6;
static recv_message_xqueue s_recv_msgs;
static context_map s_contexts;
static context_set s_pre_contexts;
static free_context_queue s_free_contexts;
static std::set<std::string> s_raw_msgs;

///
/// Running.
/// @return (0).
///
static int net_update(void);

static context_t *context_init(int idx, oid_t peer, int fd,
        const struct sockaddr_in &addr);
static context_t *context_init6(int idx, oid_t peer, int fd,
        const struct sockaddr_in6 &addr);
static void context_close(oid_t peer);
static void context_fini(context_t *ctx);
static void on_accept(const epoll_event &evt);
static void on_accept6(const epoll_event &evt);
static void on_read(const epoll_event &evt);
static void on_write(const epoll_event &evt);
static void on_error(const epoll_event &evt);

static bool is_raw_msg(const std::string &name)
{
    return s_raw_msgs.find(name) != s_raw_msgs.end();
}

static void *net_thread(void *args)
{
    while (true) {
        net_update();
        usleep(50);
    }
    return NULL;
}

static void *context_thread(void *args)
{
    while (true) {
        context_set pre_peers;
        context_set::iterator itr;

        spin_lock(&s_pre_context_lock);
        pre_peers = s_pre_contexts;
        s_pre_contexts.clear();
        spin_unlock(&s_pre_context_lock);

        for (itr = pre_peers.begin(); itr != pre_peers.end(); ++itr) {
            context_close(*itr);
        }

        time_t ct = time_s();

        while (!s_free_contexts.empty()) {
            context_t *ctx = s_free_contexts.front();

            if ((ct - ctx->close_time) < CONTEXT_CLOSE_TIME) {
                break;
            }
            context_fini(ctx);
            s_free_contexts.pop();
        }
        usleep(500);
    }
    return NULL;
}

static int net_update(void)
{
#define EPOLL_EVENT_DEFAULT_SIZE    1024

    epoll_event evts[EPOLL_EVENT_DEFAULT_SIZE];
    int num = epoll_wait(s_epoll, evts, EPOLL_EVENT_DEFAULT_SIZE, 5);

    if (num < 0 && errno != EINTR) {
        LOG_ERROR("net", "epoll_wait FAILED: %s.",
                strerror(errno));
        return num;
    }
    for (int i = 0; i < num; ++i) {
        if (evts[i].data.fd == s_sock) {
            on_accept(evts[i]);
        } else if (evts[i].data.fd == s_sock6) {
            on_accept6(evts[i]);
        } else if (evts[i].events & EPOLLIN) {
            on_read(evts[i]);
        } else if (evts[i].events & EPOLLOUT) {
            on_write(evts[i]);
        } else {
            on_error(evts[i]);
        }
    }
    return 0;
}

static void set_nonblock(int sock)
{
    // reuser socket address
    int rc = 0;
    int reuse = 1;

    rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (rc != 0) {
        LOG_ERROR("net", "setsockopt(REUSE) FAILED: %s.", strerror(errno));
        return;
    }

    // set linger
    struct linger lg;

    lg.l_onoff = LINGER_ONOFF;
    lg.l_linger = LINGER_TIME;
    rc = setsockopt(sock, SOL_SOCKET, SO_LINGER, &lg, sizeof(lg));
    if (rc != 0) {
        LOG_ERROR("net", "setsockopt(LINGER) FAILED: %s.", strerror(errno));
        return;
    }

    // set fd status
    int opts = fcntl(sock, F_GETFL);

    opts = opts | O_NONBLOCK;
    rc = fcntl(sock, F_SETFL, opts);
    if (rc != 0) {
        LOG_ERROR("net", "fcntl FAILED: %s.", strerror(errno));
        return;
    }
}

static chunk_t *chunk_init(size_t size)
{
    if (size == 0) {
        size = CHUNK_MAX_SIZE;
    }
    chunk_t *c = (chunk_t *)E_ALLOC(sizeof(chunk_t) + size);

    c->wr_offset = 0;
    c->rd_offset = 0;
    c->size = size;
    memset(c->data, 0, size);
    return c;
}

static chunk_t *chunk_init(const char *buf, size_t size)
{
    chunk_t *c = chunk_init(size);
    if (c == NULL) {
        return NULL;
    }
    memcpy(c->data, buf, size);
    return c;
}

static void chunk_fini(chunk_t *c)
{
    E_FREE(c);
}

static recv_message_t *recv_message_init(context_t *ctx)
{
    recv_message_t *msg = E_NEW recv_message_t;

    msg->peer = OID_NIL;
    msg->pb = NULL;
    msg->ctx = ctx;
    return msg;
}

static void recv_message_fini(recv_message_t *msg)
{
    assert(msg);

    E_DELETE msg->pb;
    E_DELETE msg;
}

static void chunks_push(chunk_queue &chunks, const void *buf, int size)
{
    assert(buf);

    int rem = size;
    while (rem > 0) {
        chunk_t *c = NULL;

        if (!chunks.empty()) {
            c = chunks.back();
        }
        if (c == NULL || c->wr_offset >= c->size) {
            if (size < MESSAGE_LITE_SIZE) {
                c = chunk_init(CHUNK_MIN_SIZE);
            } else {
                c = chunk_init(CHUNK_MAX_SIZE);
            }
            chunks.push_back(c);
        }

        int real = std::min(c->size - c->wr_offset, rem);

        memcpy(c->data + c->wr_offset, buf, real);
        c->wr_offset += real;
        c->size = c->wr_offset;
        rem -= real;
        buf = (char *)buf + real;
    }
}

static void message_get(chunk_queue &chunks, void *buf, int size)
{
    int offset = 0;
    int rem = size;
    chunk_queue::iterator itr = chunks.begin();

    while (rem > 0 && itr != chunks.end()) {
        chunk_t *c = *itr;

        assert(c != NULL && c->size > 0 && c->size > c->rd_offset);

        int real = c->size - c->rd_offset;
        char *src = c->data + c->rd_offset;
        char *dst = (char *)buf + offset;

        if (real > rem) {
            memcpy(dst, src, rem);
            c->rd_offset += rem;
            rem = 0;
        } else {
            memcpy(dst, src, real);
            rem -= real;
            offset += real;
            itr = chunks.erase(itr);
            chunk_fini(c);
        }
    }
}

static void message_get(chunk_queue &chunks, std::string &buf, int size)
{
    int offset = 0;
    int rem = size;
    chunk_queue::iterator itr = chunks.begin();

    while (rem > 0 && itr != chunks.end()) {
        chunk_t *c = *itr;

        assert(c != NULL && c->size > 0 && c->size > c->rd_offset);

        int real = c->size - c->rd_offset;
        char *src = c->data + c->rd_offset;

        if (real > rem) {
            buf.append(src, rem);
            c->rd_offset += rem;
            rem = 0;
        } else {
            buf.append(src, real);
            rem -= real;
            offset += real;
            itr = chunks.erase(itr);
            chunk_fini(c);
        }
    }
}

static bool message_splice(context_t *ctx)
{
    assert(ctx);

    if (ctx->recv_data->pending_size < SIZE_INTX2) return false;

    if (ctx->recv_data->pending_size > MESSAGE_MAX_PENDING_SIZE) {
        LOG_WARN("net", "%s OVER PENDING message: %d.",
                ctx->peer.info,
                ctx->recv_data->pending_size);
        net_close(ctx->peer.id);
        return false;
    }

    int &msg_size = ctx->recv_data->msg_size;

    if (msg_size == 0) {
        message_get(ctx->recv_data->chunks, &msg_size, SIZE_INT);
    }

    if (msg_size < 0 || msg_size > MESSAGE_MAX_VALID_SIZE) {
        LOG_TRACE("net", "%s INVALID message size: %d.",
                ctx->peer.info,
                msg_size);
        net_close(ctx->peer.id);
        return false;
    }

    if (ctx->recv_data->pending_size < msg_size) return false;

    int name_len = 0;
    int flag = 0;
    message_get(ctx->recv_data->chunks, &name_len, SIZE_INT);
    flag = ((name_len & ENCRYPT_FLAG) >> 30) & 0x1;
    if (flag == 1) {
        name_len ^= ENCRYPT_FLAG;
    }

    if (name_len < 0 || name_len > msg_size) {
        LOG_TRACE("net", "%s INVALID name length: %d:%d.",
                ctx->peer.info,
                msg_size, name_len);
        net_close(ctx->peer.id);
        return false;
    }

    int body_len = msg_size - SIZE_INTX2 - name_len;
    if (body_len < 0) {
        LOG_TRACE("net", "%s INVALID message size %d:%d:%d.",
                ctx->peer.info,
                msg_size, name_len, body_len);
        net_close(ctx->peer.id);
        return false;
    }

    recv_message_t *msg = recv_message_init(ctx);
    if (flag == 1)  {// encrypt
        char *name = (char *)E_ALLOC(name_len + 1);
        char *body = (char *)E_ALLOC(body_len + 1);
        memset(name, 0, name_len + 1);
        memset(body, 0, body_len + 1);
        message_get(ctx->recv_data->chunks, name, name_len);
        message_get(ctx->recv_data->chunks, body, body_len);
        cipher_t *decipher = ctx->decipher;
        if (decipher == NULL) {
            LOG_ERROR("net", "%s", "get encrypted message, but can't get decipher");
        } else {
            decipher->codec(decipher->ctx, (uint8_t*)name, (size_t)name_len);
            decipher->codec(decipher->ctx, (uint8_t*)body, (size_t)body_len);
            msg->name = std::string(name);
            msg->body = std::string(body, body_len);
        }
        E_FREE(name);
        E_FREE(body);
    } else {
        message_get(ctx->recv_data->chunks, msg->name, name_len);
        message_get(ctx->recv_data->chunks, msg->body, body_len);
    }

    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);

    ctx->recv_data->pending_size -= msg_size;
    ctx->recv_data->total_size += msg_size;
    msg_size = 0;
    return true;
}

static void blob_init(blob_t *blob)
{
    assert(blob);
    blob->chunks.clear();
    blob->msg_size = 0;
    blob->total_size = 0;
    blob->pending_size = 0;
}

static void blob_fini(blob_t *blob)
{
    assert(blob);

    chunk_queue::iterator itr = blob->chunks.begin();

    for (; itr != blob->chunks.end(); ++itr) {
        chunk_fini(*itr);
    }
    E_DELETE blob;
}

static void event_init(context_t *ctx)
{
    assert(ctx);

    epoll_event *evt = &(ctx->evt);

    memset(evt, 0, sizeof(*evt));
    evt->data.ptr = ctx;
    evt->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR;
}

static context_t *context_init(int idx, oid_t peer, int fd,
        const struct sockaddr_in &addr)
{
    context_t *ctx = E_NEW context_t;

    ctx->peer.idx = idx;
    ctx->peer.id = (peer != OID_NIL) ? peer : oid_gen();
    ctx->peer.sock = fd;
    strcpy(ctx->peer.ip, inet_ntoa(addr.sin_addr));
    ctx->peer.port = ntohs(addr.sin_port);
    sprintf(ctx->peer.info, "%d <%d>%lld (%s:%d)",
            ctx->peer.idx,
            ctx->peer.sock,
            ctx->peer.id,
            ctx->peer.ip,
            ctx->peer.port);

    ctx->last_time = ctx->start_time = time_s();
    ctx->close_time = 0;
    ctx->error_times = 0;
    ctx->recv_data = E_NEW blob_t;
    ctx->send_data = E_NEW blob_t;
    ctx->encipher = NULL;
    ctx->decipher = NULL;
    ctx->internal = false;
    blob_init(ctx->recv_data);
    blob_init(ctx->send_data);
    event_init(ctx);
    mutex_init(&ctx->lock);
    spin_lock(&s_context_lock);
    s_contexts[ctx->peer.id] = ctx;
    spin_unlock(&s_context_lock);

    recv_message_t *msg = recv_message_init(ctx);

    msg->name = "Init.Req";
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);
    return ctx;
}

static context_t *context_init6(int idx, oid_t peer, int fd,
        const struct sockaddr_in6 &addr)
{
    context_t *ctx = E_NEW context_t;

    ctx->peer.idx = idx;
    ctx->peer.id = (peer != OID_NIL) ? peer : oid_gen();
    ctx->peer.sock = fd;
    inet_ntop(AF_INET6, &addr.sin6_addr, ctx->peer.ipv6, sizeof(addr));
    ctx->peer.port = ntohs(addr.sin6_port);
    sprintf(ctx->peer.info, "%d <%d>%lld (%s:%d)",
            ctx->peer.idx,
            ctx->peer.sock,
            ctx->peer.id,
            ctx->peer.ipv6,
            ctx->peer.port);

    ctx->last_time = ctx->start_time = time_s();
    ctx->close_time = 0;
    ctx->error_times = 0;
    ctx->recv_data = E_NEW blob_t;
    ctx->send_data = E_NEW blob_t;
    ctx->encipher = NULL;
    ctx->decipher = NULL;
    ctx->internal = false;
    blob_init(ctx->recv_data);
    blob_init(ctx->send_data);
    event_init(ctx);
    mutex_init(&ctx->lock);
    spin_lock(&s_context_lock);
    s_contexts[ctx->peer.id] = ctx;
    spin_unlock(&s_context_lock);

    recv_message_t *msg = recv_message_init(ctx);

    msg->name = "Init.Req";
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);
    return ctx;
}

static void context_close(oid_t peer)
{
    context_t *ctx = NULL;

    spin_lock(&s_context_lock);
    context_map::iterator itr = s_contexts.find(peer);

    if (itr != s_contexts.end()) {
        ctx = itr->second;
        s_contexts.erase(itr);
    }
    spin_unlock(&s_context_lock);

    if (ctx == NULL) {
        return;
    }
    close(ctx->peer.sock);

    recv_message_t *msg = recv_message_init(ctx);

    msg->name = "Fini.Req";
    msg->peer = peer;
    s_recv_msgs.push(msg);

    ctx->close_time = time_s();
    s_free_contexts.push(ctx);
}

static void context_fini(context_t *ctx)
{
    assert(ctx);
    mutex_fini(&ctx->lock);
    LOG_TRACE("net", "%s is FREED. S: %d/%d R: %d/%d.",
            ctx->peer.info,
            ctx->send_data->pending_size,
            ctx->send_data->total_size,
            ctx->recv_data->pending_size,
            ctx->recv_data->total_size);
    blob_fini(ctx->send_data);
    blob_fini(ctx->recv_data);
    cipher_fini(ctx->encipher);
    cipher_fini(ctx->decipher);
    E_DELETE(ctx);
}

static context_t *context_find(oid_t peer)
{
    context_t *ctx = NULL;
    context_map::const_iterator itr;

    spin_lock(&s_context_lock);
    itr = s_contexts.find(peer);
    if (itr != s_contexts.end()) {
        ctx = itr->second;
    }
    spin_unlock(&s_context_lock);
    return ctx;
}

static void push_recv(context_t *ctx, chunk_queue &chunks)
{
    assert(ctx);

    chunk_queue::iterator itr = chunks.begin();

    for (; itr != chunks.end(); ++itr) {
        chunk_t *c = *itr;

        ctx->recv_data->pending_size += c->size;
        ctx->recv_data->chunks.push_back(c);
    }
    while (message_splice(ctx));
    chunks.clear();
}

static void push_send(context_t *ctx, blob_t *msg)
{
    assert(ctx && msg);
    mutex_lock(&(ctx->lock));
    chunk_queue::const_iterator itr = msg->chunks.begin();

    for (; itr != msg->chunks.end(); ++itr) {
        chunk_t *c = *itr;

        c->size = c->wr_offset;
        ctx->send_data->chunks.push_back(c);
    }
    msg->chunks.clear();
    ctx->send_data->pending_size += msg->total_size;
    ctx->send_data->total_size += msg->total_size;
    mutex_unlock(&(ctx->lock));

    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock, &(ctx->evt))) {
        LOG_INFO("net", "%s epoll_ctl FAILED: %s.",
            ctx->peer.info,
            strerror(errno));
    }
}

static void push_send(context_t *ctx, chunk_queue &chunks)
{
    assert(ctx);

    mutex_lock(&(ctx->lock));
    chunk_queue::const_reverse_iterator itr = chunks.rbegin();

    for (; itr != chunks.rend(); ++itr) {
        ctx->send_data->chunks.push_front(*itr);
    }
    mutex_unlock(&(ctx->lock));

    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock, &(ctx->evt))) {
        LOG_ERROR("net", "%s epoll_ctl FAILED: %s.",
            ctx->peer.info,
            strerror(errno));
    }
}

static chunk_t *pop_send(context_t *ctx, chunk_queue &clone)
{
    assert(ctx);

    chunk_t *c = NULL;
    mutex_lock(&(ctx->lock));
    chunk_queue &src = ctx->send_data->chunks;
    for (size_t i = 0; i < CHUNK_MAX_NUM && !src.empty(); ++i) {
        clone.push_back(src.front());
        src.pop_front();
    }
    mutex_unlock(&(ctx->lock));
    return c;
}

int net_init(void)
{
    MODULE_IMPORT_SWITCH;
    s_epoll = epoll_create(1000);
    if (s_epoll == -1) {
        LOG_ERROR("net", "epoll_create FAILED: %s.", strerror(errno));
        return -1;
    }
    spin_init(&s_context_lock);
    spin_init(&s_pre_context_lock);
    s_tid = thread_init(net_thread, NULL);
    s_cid = thread_init(context_thread, NULL);
    return 0;
}

int net_fini(void)
{
    MODULE_IMPORT_SWITCH;
    spin_fini(&s_pre_context_lock);
    spin_fini(&s_context_lock);
    close(s_epoll);
    return 0;
}

void net_encrypt(encrypt_func encry, encrypt_func decry)
{
}

int net_listen(const std::string &name, const std::string &ip, int port)
{
    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblock(s_sock);

    epoll_event evt;

    memset(&evt, 0, sizeof(evt));
    evt.data.fd = s_sock;
    evt.events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP;
    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, s_sock, &evt)) {
        LOG_ERROR("net", "[%s] (%s:%d) epoll_ctl FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock);
        return -1;
    }

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_aton(ip.c_str(), &(addr.sin_addr));
    addr.sin_port = htons(port);

    if (0 != bind(s_sock, (sockaddr *)&addr, sizeof(addr))) {
        LOG_ERROR("net", "[%s] (%s:%d) bind FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock);
        return -1;
    }

    // backlog: size of pending queue waiting to be accepted
    if (0 != listen(s_sock, BACKLOG)) {
        LOG_ERROR("net", "[%s] (%s:%d) listen FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock);
        return -1;
    }

    // @todo ON_LISTEN
    return 0;
}

int net_listen6(const std::string &name, const std::string &ip, int port)
{
    s_sock6 = socket(AF_INET6, SOCK_STREAM, 0);
    set_nonblock(s_sock6);

    epoll_event evt;

    memset(&evt, 0, sizeof(evt));
    evt.data.fd = s_sock6;
    evt.events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP;
    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, s_sock6, &evt)) {
        LOG_ERROR("net", "[%s] (%s:%d) epoll_ctl FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock6);
        return -1;
    }

    struct sockaddr_in6 addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip.c_str(), &(addr.sin6_addr));
    addr.sin6_port = htons(port);

    if (0 != bind(s_sock6, (sockaddr *)&addr, sizeof(addr))) {
        LOG_ERROR("net", "[%s] (%s:%d) bind FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock6);
        return -1;
    }

    // backlog: size of pending queue waiting to be accepted
    if (0 != listen(s_sock6, BACKLOG)) {
        LOG_ERROR("net", "[%s] (%s:%d) listen FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock6);
        return -1;
    }

    // @todo ON_LISTEN
    return 0;
}

int net_connect(int idx, oid_t peer, const std::string &name,
        const std::string &ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_aton(ip.c_str(), &(addr.sin_addr));
    addr.sin_port = htons(port);

    if (0 != connect(fd, (struct sockaddr *)(&addr), sizeof(addr))) {
        LOG_INFO("net", "[%s] (%s:%d) connect FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(fd);
        return -1;
    }

    // @todo ON_CONNECT
    set_nonblock(fd);
    getsockname(fd, (struct sockaddr *)(&addr), &len);
    context_t *ctx = context_init(idx, peer, fd, addr);

    ctx->internal = true;
    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
        LOG_ERROR("net", "[%s] (%s:%d) epoll_ctl FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        net_close(ctx->peer.id);
        return -1;
    }
    return 0;
}

int net_connect6(int idx, oid_t peer, const std::string &name,
        const std::string &ip, int port)
{
    int fd = socket(AF_INET6, SOCK_STREAM, 0);
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip.c_str(), &(addr.sin6_addr));
    addr.sin6_port = htons(port);

    if (0 != connect(fd, (struct sockaddr *)(&addr), sizeof(addr))) {
        LOG_INFO("net", "[%s] (%s:%d) connect FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(fd);
        return -1;
    }

    // @todo ON_CONNECT
    set_nonblock(fd);
    getsockname(fd, (struct sockaddr *)(&addr), &len);
    context_t *ctx = context_init6(idx, peer, fd, addr);

    ctx->internal = true;
    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
        LOG_ERROR("net", "[%s] (%s:%d) epoll_ctl FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        net_close(ctx->peer.id);
        return -1;
    }
    return 0;
}

void net_close(oid_t peer)
{
    if (peer <= 0) {
        return;
    }
    spin_lock(&s_pre_context_lock);
    s_pre_contexts.insert(peer);
    spin_unlock(&s_pre_context_lock);
}

int net_proc(void)
{
    recv_message_queue msgs;
    recv_message_queue::iterator itr;

    s_recv_msgs.swap(msgs);
    for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
        recv_message_t *msg = *itr;

        message_handle(msg);
        recv_message_fini(msg);
    }
    return 0;
}

static void net_stat_detail(int flag)
{
    LOG_INFO("net", "msg num: %d, msg size: %d",
            s_stat.msg_num,
            s_stat.msg_size);


    stat_msg_t *sm = NULL;
    msg_map::iterator itr;

    if (flag & NET_STAT_REQ) {
        msg_map &msgs = s_stat.req_msgs;

        for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
            sm = itr->second;
            LOG_INFO("net", "  %s> msg num: %d, msg size: %d",
                    itr->first.c_str(),
                    sm->msg_num,
                    sm->msg_size);
            E_DELETE(sm);
        }
        msgs.clear();
    }

    if (flag & NET_STAT_RES) {
        msg_map &msgs = s_stat.res_msgs;

        for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
            sm = itr->second;
            LOG_INFO("net", "  %s> msg num: %d, msg size: %d",
                    itr->first.c_str(),
                    sm->msg_num,
                    sm->msg_size);
            E_DELETE(sm);
        }
        msgs.clear();
    }
    s_stat.msg_num = 0;
    s_stat.msg_size = 0;
}

void net_stat(int flag)
{
    if (flag & NET_STAT_CONTEXTS) {
        context_map::const_iterator itr = s_contexts.begin();

        spin_lock(&s_context_lock);
        for (; itr != s_contexts.end(); ++itr) {
            context_t *ctx = itr->second;

            LOG_INFO("net", "%d %lld: RECV %d/%d SEND %d/%d.",
                    ctx->peer.idx,
                    ctx->peer.id,
                    ctx->recv_data->pending_size,
                    ctx->recv_data->total_size,
                    ctx->send_data->pending_size,
                    ctx->send_data->total_size);
        }
        spin_unlock(&s_context_lock);
    }
    net_stat_detail(flag);
}

void net_stat_message(const recv_message_t &msg)
{
    msg_map::iterator itr;

    s_stat.msg_num++;
    if (msg.pb != NULL) {
        stat_msg_t *sm = NULL;
        int size = msg.pb->ByteSize();

        s_stat.msg_size += size;
        if (msg.name.find(".Res") != std::string::npos) {
            itr = s_stat.res_msgs.find(msg.name);
            if (itr == s_stat.res_msgs.end()) {
                sm = E_NEW stat_msg_t;
                s_stat.res_msgs.insert(std::make_pair(msg.name, sm));
            } else {
                sm = itr->second;
            }
        } else if (msg.name.find(".Req") != std::string::npos) {
            itr = s_stat.req_msgs.find(msg.name);
            if (itr == s_stat.req_msgs.end()) {
                sm = E_NEW stat_msg_t;
                s_stat.req_msgs.insert(std::make_pair(msg.name, sm));
            } else {
                sm = itr->second;
            }
        } else {
            LOG_WARN("net", "INVALID message type: %s.",
                    msg.name.c_str());
            return;
        }
        sm->msg_num++;
        sm->msg_size += size;
    }
}

void net_peer_stat(oid_t peer)
{
    context_t *ctx = context_find(peer);

    if (ctx != NULL) {
        LOG_INFO("net", "%d %lld: RECV %d/%d SEND %d/%d.",
                ctx->peer.idx,
                ctx->peer.id,
                ctx->recv_data->pending_size,
                ctx->recv_data->total_size,
                ctx->send_data->pending_size,
                ctx->send_data->total_size);
    }
}

const char *net_peer_ip(const context_t *ctx)
{
    if (ctx != NULL) {
        return ctx->peer.ip;
    }
    return "0.0.0.0";
}

const char *net_peer_info(const context_t *ctx)
{
    if (ctx != NULL) {
        return ctx->peer.info;
    }
    return "INVALID PEER";
}

int net_error(context_t *ctx)
{
    if (ctx == NULL) {
        return -1;
    }
    ++(ctx->error_times);
    return 0;
}

void net_encode(const pb_t &pb, std::string &name, std::string &body)
{
    name = pb.GetTypeName();
    pb.SerializeToString(&body);
}

blob_t *net_encode(oid_t peer, const std::string &pb_name, const std::string &pb_body)
{
    context_t *ctx = context_find(peer);
    blob_t *msg = E_NEW blob_t;

    cipher_t *encipher = NULL;
    if (ctx != NULL) {
        encipher = ctx->encipher;
    }

    blob_init(msg);

    int name_len = pb_name.size();
    int body_len = pb_body.size();

    msg->total_size = name_len + body_len + SIZE_INTX2;
    chunks_push(msg->chunks, &(msg->total_size), SIZE_INT);
    if (encipher != NULL) {
        int len = name_len | ENCRYPT_FLAG;
        chunks_push(msg->chunks, &len, SIZE_INT);
    } else {
        chunks_push(msg->chunks, &name_len, SIZE_INT);
    }
    if (encipher != NULL) { // encrypt
        char *name = (char *)E_ALLOC(name_len);
        char *body = (char *)E_ALLOC(body_len);

        memcpy(name, pb_name.data(), name_len);
        memcpy(body, pb_body.data(), body_len);
        encipher->codec(encipher->ctx, (uint8_t*)name, (size_t)name_len);
        encipher->codec(encipher->ctx, (uint8_t*)body, (size_t)body_len);
        chunks_push(msg->chunks, name, name_len);
        chunks_push(msg->chunks, body, body_len);
        E_FREE(name);
        E_FREE(body);
    } else {
        const char *name = pb_name.data();
        chunks_push(msg->chunks, name, name_len);
        chunks_push(msg->chunks, pb_body.data(), body_len);
    }
    LOG_TRACE("net", "<- %s.",
            pb_name.c_str());
    return msg;
}

blob_t *net_encode(oid_t peer, const pb_t &pb)
{

    std::string name;
    std::string body;

    net_encode(pb, name, body);
    return net_encode(peer, name, body);
}

bool net_decode(recv_message_t *msg)
{
    assert(msg && msg->pb);

    context_t *ctx = msg->ctx;

    if (ctx == NULL) {
        return false;
    }

    if (!is_raw_msg(msg->name)) {
        msg->pb->ParseFromString(msg->body);
        if (!(msg->pb->IsInitialized())) {
            LOG_WARN("net", "INVALID request: %s %s.",
                    net_peer_info(ctx),
                    msg->name.c_str());
            return false;
        }
        LOG_TRACE("net", "-> %s %s.",
                net_peer_info(ctx),
                msg->name.c_str());
    }

    ctx->last_time = time_s();
    return true;
}

int net_send(oid_t peer, blob_t *msg)
{
    assert(msg);

    context_t *ctx = context_find(peer);
    if (ctx == NULL) {
        return -1;
    }
    push_send(ctx, msg);
    return 0;
}

void net_send(oid_t peer, const pb_t &pb)
{
    blob_t *msg = net_encode(peer, pb);

    net_send(peer, msg);
    blob_fini(msg);
}

void net_send(const id_set &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_set::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(*itr, name, body);
        net_send(*itr, msg);
        blob_fini(msg);
    }
}

void net_send(const obj_map_id &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    obj_map_id::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body);
        net_send(itr->first, msg);
        blob_fini(msg);
    }
}

void net_send(const pb_map_id &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    pb_map_id::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body);
        net_send(itr->first, msg);
        blob_fini(msg);
    }
}

void net_send(const id_limap &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_limap::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body);
        net_send(itr->first, msg);
        blob_fini(msg);
    }
}

void net_send(const id_ilmap &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_ilmap::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->second, name, body);
        net_send(itr->second, msg);
        blob_fini(msg);
    }
}

void net_rawsend(oid_t peer, const std::string &name, const std::string &body)
{
    blob_t *msg = net_encode(peer, name, body);
    net_send(peer, msg);
    blob_fini(msg);
}

static void on_accept(const epoll_event &evt)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int fd = 0;
    while ((fd = accept(s_sock, (sockaddr *)&addr, &len)) > 0) {
        if (0 != getpeername(fd, (sockaddr *)&addr, &len)) {
            LOG_ERROR("net", "Accept FAILED: %s.", strerror(errno));
            close(fd);
            return;
        }

        // @todo ON_ACCEPT
        set_nonblock(fd);

        context_t *ctx = context_init(0, OID_NIL, fd, addr);

        LOG_DEBUG("net", "%s", "accept new connection...");

        if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
            LOG_ERROR("net", "%s epoll_ctl FAILED: %s.",
                    ctx->peer.info,
                    strerror(errno));
            net_close(ctx->peer.id);
        }
        len = sizeof(addr);
    }
    if (fd < 0) {
        if (errno != EINTR && errno != EAGAIN) {
            LOG_ERROR("net", "Accept FAILED: %s.", strerror(errno));
        }
    }
}

static void on_accept6(const epoll_event &evt)
{
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(addr);
    int fd = 0;
    while ((fd = accept(s_sock6, (sockaddr *)&addr, &len)) > 0) {
        if (0 != getpeername(fd, (sockaddr *)&addr, &len)) {
            LOG_ERROR("net", "Accept FAILED: %s.", strerror(errno));
            close(fd);
            return;
        }

        // @todo ON_ACCEPT
        set_nonblock(fd);

        context_t *ctx = context_init6(0, OID_NIL, fd, addr);

        LOG_DEBUG("net", "%s", "accept new connection...");

        if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
            LOG_ERROR("net", "%s epoll_ctl FAILED: %s.",
                    ctx->peer.info,
                    strerror(errno));
            net_close(ctx->peer.id);
        }
        len = sizeof(addr);
    }
    if (fd < 0) {
        if (errno != EINTR && errno != EAGAIN) {
            LOG_ERROR("net", "Accept FAILED: %s.", strerror(errno));
        }
    }
}

static void on_read(const epoll_event &evt)
{
    context_t *ctx = static_cast<context_t *>(evt.data.ptr);
    int size = CHUNK_MAX_SIZE;
    int sock = ctx->peer.sock;
    chunk_queue chunks;

    static char buf[CHUNK_MAX_SIZE];
    while (size > 0) {

        size = recv(sock, buf, sizeof(buf), 0);

        if (size < 0) {
            if (errno != EINTR && errno != EAGAIN) {
                chunk_queue::iterator itr = chunks.begin();

                for (itr = chunks.begin(); itr != chunks.end(); ++itr) {
                    chunk_fini(*itr);
                }
                LOG_INFO("net", "%s recv FAILED: %s.",
                        ctx->peer.info,
                        strerror(errno));
                net_close(ctx->peer.id);
                return;
            }
            break;
        }

        // client disconnect socket
        if (0 == size) {
            chunk_queue::iterator itr = chunks.begin();

            for (itr = chunks.begin(); itr != chunks.end(); ++itr) {
                chunk_fini(*itr);
            }
            net_close(ctx->peer.id);
            return;
        }

        chunk_t *c = chunk_init(buf, size);

        // append received chunk
        c->size = size;
        chunks.push_back(c);
        if (chunks.size() > CHUNK_MAX_NUM && ctx->internal == false) {
            chunk_queue::iterator itr = chunks.begin();

            for (itr = chunks.begin(); itr != chunks.end(); ++itr) {
                chunk_fini(*itr);
            }
            net_close(ctx->peer.id);
            LOG_WARN("net", "%s <%d> chunk num over range, force closed.",
                    ctx->peer.info, chunks.size());
            return;
        }
    }
    if (chunks.size() >= 10) {
        LOG_WARN("net", "%s chunk num over range: <%d>", ctx->peer.info, chunks.size());
    }
    push_recv(ctx, chunks);
}

static void on_write(const epoll_event &evt)
{
    context_t *ctx = static_cast<context_t *>(evt.data.ptr);
    chunk_queue chunks;

    pop_send(ctx, chunks);

    int sum = 0;
    chunk_queue::iterator itr = chunks.begin();
    while (itr != chunks.end()) {
        chunk_t *c = *itr;
        int rem = c->size - c->rd_offset;

        while (rem > 0) {
            int num = send(ctx->peer.sock,
                    c->data + c->rd_offset, rem, 0);
            if (num < 0) {
                if (errno != EINTR && errno != EAGAIN) {
                    for (itr = chunks.begin(); itr != chunks.end(); ++itr) {
                        chunk_fini(*itr);
                    }
                    net_close(ctx->peer.id);
                    LOG_ERROR("net", "%s send FAILED: %s.",
                            ctx->peer.info,
                            strerror(errno));
                } else {
                    push_send(ctx, chunks);
                }
                return;
            } else {
                rem -= num;
                c->rd_offset += num;
                sum += num;
            }
        }
        chunk_fini(c);
        itr = chunks.erase(itr);
    }

    mutex_lock(&(ctx->lock));
    ctx->send_data->pending_size -= sum;
    ctx->send_data->total_size += sum;
    mutex_unlock(&(ctx->lock));
}

static void on_error(const epoll_event &evt)
{
    context_t *ctx = static_cast<context_t *>(evt.data.ptr);

    LOG_ERROR("net", "%s UNKNOWN ERROR.",
            ctx->peer.info);
    net_close(ctx->peer.id);
}

void net_cipher_set(oid_t peer, cipher_t *encipher, cipher_t *decipher)
{
    context_t *ctx = context_find(peer);
    if (ctx != NULL) {
        mutex_lock(&(ctx->lock));
        ctx->encipher = encipher;
        ctx->decipher = decipher;
        mutex_unlock(&(ctx->lock));
    }
}

void net_register_raw(const std::string &name)
{
    s_raw_msgs.insert(name);
}

void net_internal_set(oid_t peer, bool flag)
{
    context_t *ctx = context_find(peer);
    if (ctx != NULL) {
        mutex_lock(&(ctx->lock));
        ctx->internal = true;
        mutex_unlock(&(ctx->lock));
    }
}

bool net_internal(const context_t &ctx)
{
    return ctx.internal;
}
} // namespace elf
