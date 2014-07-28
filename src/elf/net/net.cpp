/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
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
#include <string>

namespace elf {
static const int CHUNK_DEFAULT_SIZE = 1024;

struct peer_t;
struct chunk_t;
struct blob_t;
struct recv_message_t;
struct context_t;

typedef std::list<chunk_t *> chunk_queue;

typedef std::deque<oid_t> context_queue;
typedef xqueue<oid_t> context_xqueue;

typedef std::deque<recv_message_t *> recv_message_queue;
typedef xqueue<recv_message_t *> recv_message_xqueue;

struct chunk_t {
    char data[CHUNK_DEFAULT_SIZE];
    int offset;
    int size;
};

struct peer_t {
    int sock;
    oid_t id;
    std::string ip;
    int port;
};

struct blob_t {
    chunk_queue chunks;
    int size;
    int msg_size;
    int total_size;
    int pending_size;
};

struct context_t {
    peer_t peer;
    mutex_t lock;
    blob_t recv_data;
    blob_t send_data;
    epoll_event evt;
    int start_time;
    int last_time;
    int error_times;
};

typedef std::map<int, id_set *> lasy_peer_map;
typedef std::map<oid_t, context_t *> context_list;
const int SIZE_INT = sizeof(int(0));
const int SIZE_INTX2 = sizeof(int(0)) * 2;

static thread_t s_tid; // io thread
static thread_t s_cid; // context thread
static mutex_t s_context_lock;
static int s_epoll;
static int s_sock;
static context_list s_contexts;
static recv_message_xqueue s_recv_msgs;
static context_xqueue s_free_contexts;
static lasy_peer_map s_lasy_peers;

///
/// Running.
/// @return (0).
///
static int net_update(void);

static context_t *context_init(oid_t peer, int fd,
        const struct sockaddr_in &addr);
static void context_fini(elf::oid_t peer);
static void on_accept(const epoll_event &evt);
static void on_read(const epoll_event &evt);
static void on_write(const epoll_event &evt);
static void on_error(const epoll_event &evt);

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
        context_queue peers;
        context_queue::iterator itr;

        s_free_contexts.swap(peers);
        if (peers.empty()) {
            usleep(500);
            continue;
        }
        LOG_TRACE("net", "%d contexts closing ...",
                peers.size());
        for (itr = peers.begin(); itr != peers.end(); ++itr) {
            context_fini(*itr);
        }
        LOG_TRACE("net", "%d contexts closed.",
                peers.size());
    }
    return NULL;
}

static int net_update(void)
{
    epoll_event evts[100];
    int num = epoll_wait(s_epoll, evts, sizeof(evts)/sizeof(evts[0]), 5);

    if (num < 0 && errno != EINTR) {
        LOG_ERROR("net", "epoll_wait FAILED: %s.",
                strerror(errno));
        return num;
    }
    for (int i = 0; i < num; ++i) {
        if (evts[i].data.fd == s_sock) {
            on_accept(evts[i]);
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

    rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse,
            sizeof(reuse));
    if (rc != 0) {
        LOG_ERROR("net", "setsockopt(REUSE) FAILED: %s.", strerror(errno));
        return;
    }

    // set linger
    struct linger lg;

    lg.l_onoff = 1;
    lg.l_linger = 10;
    rc = setsockopt(sock, SOL_SOCKET, SO_LINGER, &lg,
            sizeof(lg));
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

static chunk_t *chunk_init(void)
{
    chunk_t *c = (chunk_t *)E_ALLOC(sizeof(*c));

    c->offset = 0;
    c->size = CHUNK_DEFAULT_SIZE;
    memset(c->data, 0, CHUNK_DEFAULT_SIZE);
    return c;
}

static void chunk_fini(chunk_t *c)
{
    E_FREE(c);
}

static chunk_t *chunk_clone(const chunk_t &src)
{
    chunk_t *dst = (chunk_t *)E_ALLOC(sizeof(*dst));

    dst->size = src.offset;
    dst->offset = 0;
    memcpy(dst->data, src.data, dst->size);
    return dst;
}

static recv_message_t *recv_message_init(void)
{
    recv_message_t *msg = E_NEW recv_message_t;

    msg->peer = OID_NIL;
    msg->pb = NULL;
    return msg;
}

static void recv_message_fini(recv_message_t *msg)
{
    assert(msg);

    E_DELETE msg->pb;
    E_DELETE msg;
}

static void message_set(chunk_queue &chunks, const void *buf, int size)
{
    assert(buf);

    int rem = size;
    while (rem > 0) {
        chunk_t *c = NULL;

        if (!chunks.empty()) {
            c = chunks.back();
        }
        if (c == NULL || c->offset >= c->size) {
            c = chunk_init();
            chunks.push_back(c);
        }

        int real = std::min(c->size - c->offset, rem);

        memcpy(c->data + c->offset, buf, real);
        c->offset += real;
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
        int real = c->size - c->offset;
        char *src = c->data + c->offset;
        char *dst = (char *)buf + offset;

        if (real > rem) {
            memcpy(dst, src, rem);
            c->offset += rem;
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
        int real = c->size - c->offset;
        char *src = c->data + c->offset;

        if (real > rem) {
            buf.append(src, rem);
            c->offset += rem;
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

    if (ctx->recv_data.size < SIZE_INTX2) return false;

    int &msg_size = ctx->recv_data.msg_size;
    if (msg_size == 0) {
        message_get(ctx->recv_data.chunks, &msg_size, SIZE_INT);
    }

    if (ctx->recv_data.size < msg_size) return false;

    int name_len = 0;
    message_get(ctx->recv_data.chunks, &name_len, SIZE_INT);

    recv_message_t *msg = recv_message_init();
    int body_len = msg_size - SIZE_INTX2 - name_len;

    message_get(ctx->recv_data.chunks, msg->name, name_len);
    message_get(ctx->recv_data.chunks, msg->body, body_len);
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);
    ctx->recv_data.size -= msg_size;
    msg_size = 0;
    return true;
}

static void blob_init(blob_t *blob)
{
    assert(blob);
    blob->chunks.clear();
    blob->size = 0;
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
    evt->data.fd = -1;
    evt->data.ptr = ctx;
    evt->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR;
}

static context_t *context_init(oid_t peer, int fd,
        const struct sockaddr_in &addr)
{
    context_t *ctx = E_NEW context_t;

    ctx->peer.id = (peer != OID_NIL) ? peer : oid_gen();
    ctx->peer.sock = fd;
    ctx->peer.ip = inet_ntoa(addr.sin_addr);
    ctx->peer.port = ntohs(addr.sin_port);
    ctx->last_time = ctx->start_time = time_s();
    blob_init(&(ctx->recv_data));
    blob_init(&(ctx->send_data));
    event_init(ctx);
    mutex_init(&ctx->lock);
    mutex_lock(&s_context_lock);
    s_contexts[ctx->peer.id] = ctx;
    mutex_unlock(&s_context_lock);

    recv_message_t *msg = recv_message_init();

    msg->name = "Init.Req";
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);
    return ctx;
}

static void context_fini(elf::oid_t peer)
{
    recv_message_t *msg = recv_message_init();
    context_t *ctx = NULL;

    msg->name = "Fini.Req";
    msg->peer = peer;
    s_recv_msgs.push(msg);

    mutex_lock(&s_context_lock);
    context_list::iterator itr = s_contexts.find(peer);

    if (itr != s_contexts.end()) {
        ctx = itr->second;
        s_contexts.erase(itr);
    }
    mutex_unlock(&s_context_lock);

    if (ctx != NULL) {
        mutex_fini(&ctx->lock);
        shutdown(ctx->peer.sock, SHUT_RDWR);
        close(ctx->peer.sock);
        E_DELETE ctx;
    }
}

static context_t *context_find(oid_t peer)
{
    context_t *ctx = NULL;
    context_list::const_iterator itr;

    mutex_lock(&s_context_lock);
    itr = s_contexts.find(peer);
    if (itr != s_contexts.end()) {
        ctx = itr->second;
    }
    mutex_unlock(&s_context_lock);
    return ctx;
}

static void push_recv(context_t *ctx, chunk_t *c)
{
    assert(ctx && c);

    // @todo need lock?
    ctx->recv_data.size += c->size;
    ctx->recv_data.chunks.push_back(c);

    while (message_splice(ctx));
}

static void push_send(context_t *ctx, blob_t *msg)
{
    assert(ctx && msg);
    mutex_lock(&(ctx->lock));
    chunk_queue::const_iterator itr = msg->chunks.begin();

    for (; itr != msg->chunks.end(); ++itr) {
        chunk_t *c = chunk_clone(**itr);

        ctx->send_data.chunks.push_back(c);
    }
    ctx->send_data.pending_size += msg->size;
    ctx->send_data.total_size += msg->size;
    mutex_unlock(&(ctx->lock));

    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock,
                &(ctx->evt))) {
        LOG_ERROR("net", "%lld (%s:%d) epoll_ctl FAILED: %s.",
                ctx->peer.id,
                ctx->peer.ip.c_str(), ctx->peer.port,
                strerror(errno));
    }
}

static void push_send(context_t *ctx, chunk_queue &chunks)
{
    assert(ctx);

    mutex_lock(&(ctx->lock));
    chunk_queue::const_reverse_iterator itr = chunks.rbegin();

    for (; itr != chunks.rend(); ++itr) {
        ctx->send_data.chunks.push_front(*itr);
    }
    mutex_unlock(&(ctx->lock));

    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock,
                &(ctx->evt))) {
        LOG_ERROR("net", "%lld (%s:%d) epoll_ctl FAILED: %s.",
                ctx->peer.id,
                ctx->peer.ip.c_str(), ctx->peer.port,
                strerror(errno));
    }
}

static chunk_t *pop_send(context_t *ctx, chunk_queue &clone)
{
    assert(ctx);

    chunk_t *c = NULL;
    mutex_lock(&(ctx->lock));
    clone = ctx->send_data.chunks;
    ctx->send_data.chunks.clear();
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
    mutex_init(&s_context_lock);
    s_tid = thread_init(net_thread, NULL);
    s_cid = thread_init(context_thread, NULL);
    return 0;
}

int net_fini(void)
{
    MODULE_IMPORT_SWITCH;
    mutex_fini(&s_context_lock);
    close(s_epoll);
    return 0;
}

int net_listen(oid_t peer, const std::string &name,
        const std::string &ip, int port)
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

    if (0 != listen(s_sock, 0)) {
        LOG_ERROR("net", "[%s] (%s:%d) listen FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(s_sock);
        return -1;
    }
    LOG_INFO("net", "[%s] (%s:%d) listen OK.",
            name.c_str(), ip.c_str(), port);

    // @todo ON_LISTEN
    return 0;
}

int net_connect(oid_t peer, const std::string &name,
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
        LOG_ERROR("net", "[%s] (%s:%d) connect FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        close(fd);
        return -1;
    }

    // @todo ON_CONNECT
    set_nonblock(fd);
    getsockname(fd, (struct sockaddr *)(&addr), &len);
    context_t *ctx = context_init(peer, fd, addr);

    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
        LOG_ERROR("net", "[%s] (%s:%d) epoll_ctl FAILED: %s.",
                name.c_str(), ip.c_str(), port,
                strerror(errno));
        net_close(ctx->peer.id);
        return -1;
    } else {
        LOG_INFO("net", "[%s] (%s:%d) connect OK.",
               name.c_str(), ip.c_str(), port);
    }
    return 0;
}

void net_close(oid_t peer)
{
    s_free_contexts.push(peer);
}

int net_proc(void)
{
    recv_message_queue msgs;
    recv_message_queue::iterator itr;

    s_recv_msgs.swap(msgs);
    for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
        message_handle(*itr);
        recv_message_fini(*itr);
    }
    return 0;
}

void net_stat(void)
{
    mutex_lock(&s_context_lock);
    LOG_INFO("net", "%d clients connected.",
            s_contexts.size());
    mutex_unlock(&s_context_lock);
}

void net_peer_addr(oid_t peer, char *str)
{
    context_t *ctx = context_find(peer);
    if (ctx == NULL) {
        LOG_TRACE("net", "%lld is NOT found.",
                peer);
        return;
    }
    sprintf(str, "%s:%d",
            ctx->peer.ip.c_str(), ctx->peer.port);
}

void net_peer_info(oid_t peer, char *str)
{
    context_t *ctx = context_find(peer);
    if (ctx == NULL) {
        LOG_TRACE("net", "%lld is NOT found.",
                peer);
        return;
    }
    sprintf(str, "%lld (%s:%d)",
            ctx->peer.id,
            ctx->peer.ip.c_str(), ctx->peer.port);
}

int net_error(oid_t peer)
{
    context_t *ctx = context_find(peer);
    if (ctx == NULL) {
        LOG_TRACE("net", "%lld is NOT found.",
                peer);
        return -1;
    }
    ++(ctx->error_times);
    return 0;
}

blob_t *net_encode(const pb_t &pb)
{
    std::string buf;
    blob_t *msg = E_NEW blob_t;

    blob_init(msg);
    pb.SerializeToString(&buf);

    int name_len = pb.GetTypeName().size();
    int body_len = buf.size();

    msg->size = name_len + body_len + SIZE_INTX2;
    message_set(msg->chunks, &(msg->size), SIZE_INT);
    message_set(msg->chunks, &name_len, SIZE_INT);
    message_set(msg->chunks, pb.GetTypeName().data(), name_len);
    message_set(msg->chunks, buf.data(), body_len);
    return msg;
}

bool net_decode(recv_message_t *msg)
{
    assert(msg && msg->pb);
    msg->pb->ParseFromString(msg->body);

    context_t *ctx = context_find(msg->peer);
    if (ctx == NULL) {
        LOG_TRACE("net", "%lld is NOT found.",
                msg->peer);
        return false;
    }
    ctx->last_time = elf::time_s();
    return true;
}

int net_send(oid_t peer, blob_t *msg)
{
    assert(msg);

    context_t *ctx = context_find(peer);
    if (ctx == NULL) {
        LOG_TRACE("net", "%lld is NOT found to be sent.",
                peer);
        return -1;
    }

    push_send(ctx, msg);
    return 0;
}

void net_send(oid_t peer, const pb_t &pb)
{
    blob_t *msg = net_encode(pb);

    net_send(peer, msg);
    blob_fini(msg);
}

void net_send(const id_set &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    blob_t *msg = net_encode(pb);
    id_set::const_iterator itr = peers.begin();

    for (; itr != peers.end(); ++itr) {
        net_send(*itr, msg);
    }
    blob_fini(msg);
}

void net_send(const obj_map_id &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    blob_t *msg = net_encode(pb);
    obj_map_id::const_iterator itr = peers.begin();

    for (; itr != peers.end(); ++itr) {
        net_send(itr->first, msg);
    }
    blob_fini(msg);
}

void net_send(const id_map &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    blob_t *msg = net_encode(pb);
    id_map::const_iterator itr = peers.begin();

    for (; itr != peers.end(); ++itr) {
        net_send(itr->first, msg);
    }
    blob_fini(msg);
}

void net_send(const id_imap &peers, const pb_t &pb)
{
    if (peers.empty()) return;

    blob_t *msg = net_encode(pb);
    id_imap::const_iterator itr = peers.begin();

    for (; itr != peers.end(); ++itr) {
        net_send(itr->second, msg);
    }
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

        context_t *ctx = context_init(OID_NIL, fd, addr);

        if (0 != epoll_ctl(s_epoll, EPOLL_CTL_ADD, fd, &(ctx->evt))) {
            LOG_ERROR("net", "%lld (%s:%d) epoll_ctl FAILED: %s.",
                    ctx->peer.id,
                    ctx->peer.ip.c_str(), ctx->peer.port,
                    strerror(errno));
            net_close(ctx->peer.id);
        } else {
            LOG_INFO("net", "%lld (%s:%d) accepted.",
                    ctx->peer.id,
                    ctx->peer.ip.c_str(), ctx->peer.port);
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
    context_t *ctx = (context_t *)(evt.data.ptr);
    int size = CHUNK_DEFAULT_SIZE;

    while (size > 0) {
        chunk_t *c = chunk_init();

        size = recv(ctx->peer.sock, c->data, sizeof(c->data), 0);
        if (size < 0) {
            chunk_fini(c);
            if (errno != EINTR && errno != EAGAIN) {
                LOG_ERROR("net", "%lld (%s:%d) recv FAILED: %s.",
                        ctx->peer.id,
                        ctx->peer.ip.c_str(), ctx->peer.port,
                        strerror(errno));
                net_close(ctx->peer.id);
                return;
            }
            break;
        }

        // client disconnect socket
        if (0 == size) {
            chunk_fini(c);
            LOG_INFO("net", "%lld (%s:%d) active closed.",
                    ctx->peer.id,
                    ctx->peer.ip.c_str(), ctx->peer.port);
            net_close(ctx->peer.id);
            return;
        }

        // append received chunk
        c->size = size;
        push_recv(ctx, c);
    }
    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock,
                &(ctx->evt))) {
        LOG_ERROR("net", "%lld (%s:%d) epoll_ctl FAILED: %s.",
                ctx->peer.id,
                ctx->peer.ip.c_str(), ctx->peer.port,
                strerror(errno));
    }
}

static void on_write(const epoll_event &evt)
{
    chunk_queue chunks;
    context_t *ctx = (context_t *)(evt.data.ptr);

    pop_send(ctx, chunks);

    int total = 0;
    chunk_queue::iterator itr = chunks.begin();
    while (itr != chunks.end()) {
        chunk_t *c = *itr;
        int rem = c->size - c->offset;

        while (rem > 0) {
            int num = send(ctx->peer.sock,
                    c->data + c->offset, rem, 0);
            if (num < 0) {
                if (errno != EINTR && errno != EAGAIN) {
                    LOG_ERROR("net", "%lld (%s:%d) send FAILED: %s.",
                            ctx->peer.id,
                            ctx->peer.ip.c_str(), ctx->peer.port,
                            strerror(errno));
                    net_close(ctx->peer.id);
                } else {
                    push_send(ctx, chunks);
                }
                return;
            } else {
                rem -= num;
                c->offset += num;
                total += num;
                ctx->send_data.pending_size -= num;
                ctx->send_data.size += num;
            }
        }
        chunk_fini(c);
        itr = chunks.erase(itr);
    }
    mutex_lock(&(ctx->lock));
    mutex_unlock(&(ctx->lock));
}

static void on_error(const epoll_event &evt)
{
    context_t *ctx = (context_t *)(evt.data.ptr);

    LOG_ERROR("net", "%lld (%s:%d), UNKNOWN ERROR.",
            ctx->peer.id,
            ctx->peer.ip.c_str(), ctx->peer.port);
    ctx->evt.events = EPOLLIN|EPOLLET;
    epoll_ctl(s_epoll, EPOLL_CTL_DEL, ctx->peer.sock, &(ctx->evt));
    net_close(ctx->peer.id);
}
} // namespace elf
