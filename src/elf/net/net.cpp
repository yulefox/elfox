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
#include <elf/os.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <string>

namespace elf {
static const int LINGER_ONOFF = 0;
static const int LINGER_TIME = 5;
static const int CONTEXT_CLOSE_TIME = 90;
static const int SIZE_INT = sizeof(int(0));
static const int SIZE_INTX2 = sizeof(int(0)) * 2;
static const int CHUNK_SIZE_S = 256;
static const int CHUNK_SIZE_L = 4096;
static const size_t CHUNK_MAX_NUM = 8192;
static const int MESSAGE_MAX_NAME_LENGTH = 100;
static const int MESSAGE_MAX_VALID_SIZE = CHUNK_MAX_NUM * CHUNK_SIZE_L;
static const int MESSAGE_MAX_PENDING_SIZE = MESSAGE_MAX_VALID_SIZE * 2;
static const int BACKLOG = 128;
static const int NAME_SIZE_MASK = 0xFFFFFF;
static const int NAME_SIZE_BITS = 24;
static const int DEFAULT_WORKER_THREAD_SIZE = 4;
static const int CONTEXT_FREE_THRESHOLD = 1024;

enum MSG_FLAG {
    ENCRYPT_FLAG    = 0x1,
    RAW_FLAG        = 0x2,
};


struct blob_t;
struct chunk_t;
struct context_t;
struct peer_t;
struct recv_message_t;
struct stat_msg_t;

typedef std::list<chunk_t *> chunk_queue;
typedef std::map<std::string, stat_msg_t *> msg_map;
typedef std::deque<recv_message_t *> recv_message_queue;
typedef xqueue<context_t *> free_context_queue;
typedef xqueue<recv_message_t *> recv_message_xqueue;
typedef xqueue<context_t *> context_xqueue;
typedef xqueue<blob_t*> write_context_xqueue;

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
    int send_msg_num; // number of send msg
    int send_msg_size; // size of send msg
    int recv_msg_num; // number of recv msg
    int recv_msg_size; // size of recv msg
    msg_map req_msgs; // request message map
    msg_map res_msgs; // response message map
    size_t context_size_created;
    size_t context_size_released;
    size_t chunk_size_created;
    size_t chunk_size_released;

    stat_t() :
        send_msg_num(0), 
        send_msg_size(0),
        recv_msg_num(0), 
        recv_msg_size(0),
        context_size_created(0),
        context_size_released(0),
        chunk_size_created(0),
        chunk_size_released(0)
    {
    }
};

static stat_t s_stat;

struct blob_t {
    chunk_queue chunks;
    int msg_size; // current recv msg size(for splicing)
    int total_size; // total send/recv msg size
    int pending_size; // pending send/recv msg size
    void *ctx;
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
    int ref_cnt;
    bool internal;
    int running;
    int instance;
    int worker_idx;

    context_t()
    {
        ++s_stat.context_size_created;
    }

    ~context_t()
    {
        ++s_stat.context_size_released;
    }

    int inc_ref()
    {
        return __sync_add_and_fetch(&ref_cnt, 1);
    }

    int dec_ref()
    {
        return __sync_sub_and_fetch(&ref_cnt, 1);
    }

    int ref()
    {
        return __sync_val_compare_and_swap(&ref_cnt, 0, 0);
    }

    int alive()
    {
        return __sync_val_compare_and_swap(&running, 1, 1);
    }
};

typedef std::map<oid_t, context_t *> context_map;
typedef std::set<oid_t> context_set;

static thread_t s_tid; // io thread
static thread_t s_cid; // context thread
static thread_t *s_writer_tid; // writer thread
static thread_t *s_reader_tid; // reader thread

static context_xqueue *s_pending_read;
static write_context_xqueue *s_pending_write;

static spin_t s_context_lock;
static spin_t s_pre_context_lock;
static int s_epoll;
static int s_sock;
static int s_sock6;
static int s_next_worker = 0;
static int s_worker_num;
static recv_message_xqueue s_recv_msgs;
static context_map s_contexts;
static context_set s_pre_contexts;
static free_context_queue s_free_contexts;
static std::set<std::string> s_raw_msgs;
static std::list<context_t*> s_pending_gc;

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
static bool context_fini(context_t *ctx);
static void context_stop(context_t *ctx);
static void on_accept(const epoll_event &evt);
static void on_accept6(const epoll_event &evt);
static void on_read(const epoll_event &evt);
static void on_write(const epoll_event &evt);
static void handle_read(context_t *ctx);
static bool handle_write(context_t *ctx);
static void on_error(const epoll_event &evt);
static void append_send(context_t *ctx, blob_t *msg);
static void blob_init(blob_t *blob, context_t *ctx);
static void blob_fini(blob_t *blob);


static bool is_raw_msg(const std::string &name)
{
    return s_raw_msgs.find(name) != s_raw_msgs.end();
}

static void *net_thread(void *args)
{
    while (true) {
        net_update();
    }
    return NULL;
}

static void context_stop(context_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    if (0 == __sync_val_compare_and_swap(&(ctx->running), 1, 0)) {
        return;
    }

    LOG_DEBUG("net", "context stop: %lld %d %d", ctx->peer.id, ctx->peer.sock, ctx->internal);

    // do notify wthread
    blob_t *msg = E_NEW blob_t;
    blob_init(msg, ctx);
    s_pending_write[ctx->worker_idx].push(msg);
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

        for (std::list<context_t*>::iterator itr = s_pending_gc.begin(); itr != s_pending_gc.end(); ) {
            context_t *ctx = *itr;
            if (ctx == NULL) {
                itr = s_pending_gc.erase(itr);
            } else {
                if (context_fini(ctx)) {
                    itr = s_pending_gc.erase(itr);
                } else {
                    ++itr;
                }
            }
        }

        // free
        while (s_free_contexts.size() > CONTEXT_FREE_THRESHOLD) {
            context_t *ctx = NULL;
            s_free_contexts.pop(ctx);
            if (ctx != NULL) {
                E_DELETE ctx;
            }
        }
        usleep(50000);
    }
    return NULL;
}


static void *net_reader(void *args)
{
    context_xqueue *que = (context_xqueue *)(args);
    while (true) {
        std::deque<context_t*> ctxs;
        std::deque<context_t*>::iterator itr;
        que->swap(ctxs);
        for (itr = ctxs.begin(); itr != ctxs.end(); ++itr) {
            handle_read(*itr);
        }
        usleep(50000);
    }
    return NULL;
}

static void *net_writer(void *args)
{
    write_context_xqueue *que = (write_context_xqueue *)(args);

    while (true) {
        blob_t *msg = NULL;
        que->pop(msg);

        ///
        int instance;
        context_t *ctx = (context_t*)(msg->ctx);
        instance = (uintptr_t)ctx & 1;
        ctx = (context_t*)((uintptr_t)ctx & (uintptr_t) ~1);
        if (ctx->peer.sock == -1 || ctx->instance != instance) {
            LOG_DEBUG("net", "%s", "expired send req...");
            blob_fini(msg);
            continue;
        }

        if (ctx->alive() == 1) {
            if (msg != NULL && msg->total_size > 0) {
                append_send(ctx, msg);
            }
            handle_write(ctx);
        }
        blob_fini(msg);

        // release
        if (ctx->alive() == 0) {
            net_close(ctx->peer.id);
        }
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

    int enable = 1;
    rc = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    if (rc != 0) {
        LOG_ERROR("net", "setsockopt(TCP_NODELAY) FAILED: %s.", strerror(errno));
        return;
    }
}

static chunk_t *chunk_init(size_t size)
{
    size = (size <= CHUNK_SIZE_S) ? CHUNK_SIZE_S : CHUNK_SIZE_L;

    chunk_t *c = (chunk_t *)E_ALLOC(sizeof(chunk_t) + size);

    c->wr_offset = 0;
    c->rd_offset = 0;
    c->size = size;
    memset(c->data, 0, size);
    ++s_stat.chunk_size_created;
    return c;
}

static chunk_t *chunk_init(const char *buf, size_t size)
{
    assert(buf && size <= CHUNK_SIZE_L);

    chunk_t *c = chunk_init(size);

    if (c == NULL) {
        return NULL;
    }
    memcpy(c->data, buf, size);
    c->wr_offset = size;
    return c;
}

static void chunk_fini(chunk_t *c)
{
    E_FREE(c);
    ++s_stat.chunk_size_released;
}

static recv_message_t *recv_message_init(context_t *ctx)
{
    recv_message_t *msg = E_NEW recv_message_t;

    msg->peer = 0;
    msg->dest = 0;
    msg->pb = NULL;
    msg->ctx = ctx;
    msg->rpc_ctx = NULL;
    msg->is_raw = false;
    ctx->inc_ref();
    return msg;
}

static void recv_message_fini(recv_message_t *msg)
{
    assert(msg);

    //
    if (msg->ctx != NULL) {
        msg->ctx->dec_ref();
    }

    S_DELETE(msg->pb);
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
            c = chunk_init(size);
            chunks.push_back(c);
        }

        int real = std::min(c->size - c->wr_offset, rem);

        memcpy(c->data + c->wr_offset, buf, real);
        c->wr_offset += real;
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
        int real = c->wr_offset - c->rd_offset;

        assert(c != NULL && real > 0);

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
        int real = c->wr_offset - c->rd_offset;

        assert(c != NULL && real > 0);

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
        context_stop(ctx);
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
        context_stop(ctx);
        return false;
    }

    if (ctx->recv_data->pending_size < msg_size) return false;

    int name_len = 0;
    int flag = 0;
    message_get(ctx->recv_data->chunks, &name_len, SIZE_INT);

    flag = name_len >> NAME_SIZE_BITS;
    name_len = name_len & NAME_SIZE_MASK;

    if (name_len < 0 || name_len > msg_size) {
        LOG_TRACE("net", "%s INVALID name length: %d:%d.",
                ctx->peer.info,
                msg_size, name_len);
        context_stop(ctx);
        return false;
    }

    int body_len = msg_size - SIZE_INTX2 - name_len;
    if (flag & RAW_FLAG) {
        body_len -= sizeof(oid_t);
    }
    if (body_len < 0) {
        LOG_TRACE("net", "%s INVALID message size %d:%d:%d.",
                ctx->peer.info,
                msg_size, name_len, body_len);
        context_stop(ctx);
        return false;
    }

    recv_message_t *msg = recv_message_init(ctx);
    if (flag & ENCRYPT_FLAG) {// encrypt
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

    if (flag & RAW_FLAG) {
        oid_t cid = 0;
        message_get(ctx->recv_data->chunks, &cid, sizeof(oid_t));

        msg->dest = cid;
    }

    msg->is_raw = is_raw_msg(msg->name);
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);

    ctx->recv_data->pending_size -= msg_size;
    ctx->recv_data->total_size += msg_size;
    msg_size = 0;
    return true;
}

static void blob_init(blob_t *blob, context_t *ctx)
{
    assert(blob);
    blob->chunks.clear();
    blob->msg_size = 0;
    blob->total_size = 0;
    blob->pending_size = 0;
    blob->ctx = NULL;

    if (ctx != NULL) {
        ctx->inc_ref();
        blob->ctx = (void*)((uintptr_t)ctx | ctx->instance);
    }
}

static void blob_fini(blob_t *blob)
{
    assert(blob);

    if (blob->ctx != NULL) {
        context_t *ctx = (context_t*)(blob->ctx);
        //int instance = (uintptr_t)ctx & 1;
        ctx = (context_t*)((uintptr_t)ctx & (uintptr_t) ~1);
        ctx->dec_ref();
    }

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
    evt->data.ptr = (void*)((uintptr_t)ctx | ctx->instance);
    evt->events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLERR;
}


static context_t *context_alloc()
{
    context_t *ctx = NULL;

    if (s_free_contexts.size() > 0) {
        s_free_contexts.pop(ctx);
    } else {
        ctx = E_NEW context_t;
        ctx->instance = 0;
    }

    //
    ctx->worker_idx = __sync_fetch_and_add(&s_next_worker, 1) % s_worker_num;
    return ctx;
}

static context_t *context_init(int idx, oid_t peer, int fd,
        const struct sockaddr_in &addr)
{
    context_t *ctx = context_alloc();

    ctx->peer.idx = idx;
    ctx->peer.id = (peer != 0) ? peer : oid_gen();
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
    ctx->running = 1;
    ctx->ref_cnt = 1;
    int instance = ctx->instance;
    ctx->instance = !instance;
    blob_init(ctx->recv_data, NULL);
    blob_init(ctx->send_data, NULL);
    event_init(ctx);
    mutex_init(&ctx->lock);
    spin_lock(&s_context_lock);
    s_contexts[ctx->peer.id] = ctx;
    spin_unlock(&s_context_lock);

    recv_message_t *msg = recv_message_init(ctx);

    msg->name = "msg.Init.Req";
    msg->peer = ctx->peer.id;
    s_recv_msgs.push(msg);

    return ctx;
}

static context_t *context_init6(int idx, oid_t peer, int fd,
        const struct sockaddr_in6 &addr)
{
    context_t *ctx = context_alloc();

    ctx->peer.idx = idx;
    ctx->peer.id = (peer != 0) ? peer : oid_gen();
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
    ctx->running = 1;
    ctx->ref_cnt = 1;
    int instance = ctx->instance;
    ctx->instance = !instance;
    blob_init(ctx->recv_data, NULL);
    blob_init(ctx->send_data, NULL);
    event_init(ctx);
    mutex_init(&ctx->lock);
    spin_lock(&s_context_lock);
    s_contexts[ctx->peer.id] = ctx;
    spin_unlock(&s_context_lock);

    recv_message_t *msg = recv_message_init(ctx);

    msg->name = "msg.Init.Req";
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

    msg->name = "msg.Fini.Req";
    msg->peer = peer;
    s_recv_msgs.push(msg);

    ctx->close_time = time_s();
    //context_fini(ctx);
    //
    ctx->dec_ref();

    //
    s_pending_gc.push_back(ctx);
}

static bool context_fini(context_t *ctx)
{
    assert(ctx);

    if (__sync_val_compare_and_swap(&(ctx->ref_cnt), 0, 0) > 0) {
        return false;
    }

    mutex_fini(&ctx->lock);

    LOG_INFO("net", "%s is FREED. S: %d/%d R: %d/%d.",
            ctx->peer.info,
            ctx->send_data->pending_size,
            ctx->send_data->total_size,
            ctx->recv_data->pending_size,
            ctx->recv_data->total_size);
    blob_fini(ctx->send_data);
    blob_fini(ctx->recv_data);
    cipher_fini(ctx->encipher);
    cipher_fini(ctx->decipher);

    ctx->send_data = NULL;
    ctx->recv_data = NULL;
    ctx->peer.sock = -1;
    ctx->peer.id = 0;
    //E_DELETE(ctx);

    // gc
    s_free_contexts.push(ctx);
    return true;
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

        ctx->recv_data->pending_size += (c->wr_offset - c->rd_offset);
        ctx->recv_data->chunks.push_back(c);
    }
    while (message_splice(ctx));
    chunks.clear();
}

static void push_send(context_t *ctx, blob_t *msg)
{
    if (ctx->alive() == 1) {
        s_pending_write[ctx->worker_idx].push(msg);
    }
}

static void append_send(context_t *ctx, blob_t *msg)
{
    assert(ctx && msg);
    chunk_queue::const_iterator itr = msg->chunks.begin();
    for (; itr != msg->chunks.end(); ++itr) {
        chunk_t *c = *itr;
        ctx->send_data->chunks.push_back(c);
    }
    msg->chunks.clear();
    ctx->send_data->pending_size += msg->total_size;
    ctx->send_data->total_size += msg->total_size;

    ++s_stat.send_msg_num;
    s_stat.send_msg_size += msg->total_size;
}

int net_init(int worker_num)
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

    s_worker_num = worker_num;
    if (s_worker_num <= 0) {
        s_worker_num = get_cpus();
    }
    if (s_worker_num <= 0) {
        s_worker_num = DEFAULT_WORKER_THREAD_SIZE;
    }
    LOG_INFO("net", "worker num: %d, cpus: %d", s_worker_num, get_cpus());

    s_writer_tid = E_NEW thread_t[s_worker_num];
    s_reader_tid = E_NEW thread_t[s_worker_num];
    s_pending_read = E_NEW context_xqueue[s_worker_num];
    s_pending_write = E_NEW write_context_xqueue[s_worker_num];
    for (int i = 0; i < s_worker_num; i++) {
        s_writer_tid[i] = thread_init(net_writer, s_pending_write + i);
        s_reader_tid[i] = thread_init(net_reader, s_pending_read + i);
    }

    return 0;
}

int net_fini(void)
{
    MODULE_IMPORT_SWITCH;
    spin_fini(&s_pre_context_lock);
    spin_fini(&s_context_lock);
    close(s_epoll);
    E_DELETE s_writer_tid;
    E_DELETE s_reader_tid;
    E_DELETE s_pending_read;
    E_DELETE s_pending_write;
    return 0;
}

void net_encrypt(encrypt_func encry, encrypt_func decry)
{
}

int net_listen(const std::string &name, const std::string &ip, int port)
{
    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblock(s_sock);

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

    // @todo ON_LISTEN
    return 0;
}

int net_listen6(const std::string &name, const std::string &ip, int port)
{
    s_sock6 = socket(AF_INET6, SOCK_STREAM, 0);
    set_nonblock(s_sock6);

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
    LOG_INFO("stat", "send msg: %d(%d), recv msg: %d(%d), contexts: %d/%d, chunks: %d/%d",
            s_stat.send_msg_num,
            s_stat.send_msg_size,
            s_stat.recv_msg_num,
            s_stat.recv_msg_size,
            s_stat.context_size_created,
            s_stat.context_size_released,
            s_stat.chunk_size_created,
            s_stat.chunk_size_released);

    stat_msg_t *sm = NULL;
    msg_map::iterator itr;

    if (flag & NET_STAT_REQ) {
        msg_map &msgs = s_stat.req_msgs;

        for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
            sm = itr->second;
            LOG_INFO("stat", "  %s> msg num: %d, msg size: %d",
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
            LOG_INFO("stat", "  %s> msg num: %d, msg size: %d",
                    itr->first.c_str(),
                    sm->msg_num,
                    sm->msg_size);
            E_DELETE(sm);
        }
        msgs.clear();
    }
    s_stat.send_msg_num = 0;
    s_stat.send_msg_size = 0;
    s_stat.recv_msg_num = 0;
    s_stat.recv_msg_size = 0;
}

void net_stat(int flag)
{
    net_stat_detail(flag);

    if (flag & NET_STAT_CONTEXTS) {
        context_map::const_iterator itr = s_contexts.begin();

        spin_lock(&s_context_lock);
        for (; itr != s_contexts.end(); ++itr) {
            context_t *ctx = itr->second;

            LOG_INFO("stat", "%d %lld: RECV %d/%d SEND %d/%d.",
                    ctx->peer.idx,
                    ctx->peer.id,
                    ctx->recv_data->pending_size,
                    ctx->recv_data->total_size,
                    ctx->send_data->pending_size,
                    ctx->send_data->total_size);
        }
        spin_unlock(&s_context_lock);
    }
}

void net_stat_message(const recv_message_t &msg)
{
    msg_map::iterator itr;

    if (msg.pb != NULL) {
        stat_msg_t *sm = NULL;
        int size = msg.pb->ByteSize();

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
        s_stat.recv_msg_num++;
        s_stat.recv_msg_size += size;
    }
}

void net_peer_stat(oid_t peer)
{
    context_t *ctx = context_find(peer);

    if (ctx != NULL) {
        LOG_INFO("stat", "%d %lld: RECV %d/%d SEND %d/%d.",
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

blob_t *net_encode(oid_t peer, const std::string &pb_name, const std::string &pb_body, oid_t dest)
{
    context_t *ctx = context_find(peer);
    blob_t *msg = E_NEW blob_t;

    cipher_t *encipher = NULL;
    if (ctx != NULL) {
        encipher = ctx->encipher;
    }

    blob_init(msg, ctx);

    int name_len = pb_name.size();
    int body_len = pb_body.size();
    int flag = 0;

    if (dest > 0) {
        flag |= RAW_FLAG;
    }
    if (encipher != NULL) {
        flag |= ENCRYPT_FLAG;
    }

    msg->total_size = name_len + body_len + SIZE_INTX2;
    if (flag & RAW_FLAG) {
        msg->total_size += sizeof(dest);
    }

    // msg total size
    chunks_push(msg->chunks, &(msg->total_size), SIZE_INT);

    // namesize + flag
    int val = (flag << NAME_SIZE_BITS) | name_len;
    chunks_push(msg->chunks, &val, SIZE_INT);

    if (flag & ENCRYPT_FLAG) { // encrypt
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

    // push cid
    if (flag & RAW_FLAG) {
        chunks_push(msg->chunks, &dest, sizeof(dest));
    }
    LOG_TRACE("net", "<- %s.",
            pb_name.c_str());
    return msg;
}

blob_t *net_encode(oid_t peer, const pb_t &pb, oid_t dest)
{

    std::string name;
    std::string body;

    net_encode(pb, name, body);
    return net_encode(peer, name, body, dest);
}

bool net_decode(recv_message_t *msg)
{
    //assert(msg && msg->pb);
    assert(msg);

    context_t *ctx = msg->ctx;

    if (ctx == NULL) {
        return false;
    }

    if (!msg->is_raw) {
        bool flag = msg->pb->ParseFromString(msg->body);
        if (flag == false) {
            LOG_WARN("net", "Protobuf parsing FAILED: %s %s.",
                    net_peer_info(ctx),
                    msg->name.c_str());
            return false;
        }
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

void net_send(oid_t peer, const pb_t &pb, oid_t dest)
{
    blob_t *msg = net_encode(peer, pb, dest);

    net_send(peer, msg);
    //blob_fini(msg);
}

void net_send(const id_set &peers, const pb_t &pb, oid_t dest)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_set::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(*itr, name, body, dest);
        net_send(*itr, msg);
        //blob_fini(msg);
    }
}

void net_send(const obj_map_id &peers, const pb_t &pb, oid_t dest)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    obj_map_id::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body, dest);
        net_send(itr->first, msg);
        //blob_fini(msg);
    }
}

void net_send(const pb_map_id &peers, const pb_t &pb, oid_t dest)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    pb_map_id::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body, dest);
        net_send(itr->first, msg);
        //blob_fini(msg);
    }
}

void net_send(const id_limap &peers, const pb_t &pb, oid_t dest)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_limap::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->first, name, body, dest);
        net_send(itr->first, msg);
        //blob_fini(msg);
    }
}

void net_send(const id_ilmap &peers, const pb_t &pb, oid_t dest)
{
    if (peers.empty()) return;

    std::string name;
    std::string body;
    net_encode(pb, name, body);

    id_ilmap::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        blob_t *msg = net_encode(itr->second, name, body, dest);
        net_send(itr->second, msg);
        //blob_fini(msg);
    }
}

void net_rawsend(oid_t peer, const std::string &name, const std::string &body, oid_t dest)
{
    blob_t *msg = net_encode(peer, name, body, dest);
    net_send(peer, msg);
    //blob_fini(msg);
}

void net_rawsend(const id_set &peers, const std::string &name, const std::string &body, oid_t dest)
{
    if (peers.empty()) return;

    id_set::const_iterator itr = peers.begin();
    for (; itr != peers.end(); ++itr) {
        net_rawsend(*itr, name, body, dest);
    }
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

        context_t *ctx = context_init(0, 0, fd, addr);

        LOG_INFO("net", "accept new connection fd=%d...", fd);

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

        context_t *ctx = context_init6(0, 0, fd, addr);

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
    context_t *ctx = (context_t*)(evt.data.ptr);
    if (ctx == NULL) {
        return;
    }

    int instance = (uintptr_t)ctx & 1;
    ctx = (context_t*)((uintptr_t)ctx & (uintptr_t) ~1);
    if (ctx->alive() == 0 || ctx->peer.sock < 0 || ctx->instance != instance) {
        LOG_DEBUG("net", "%s", "expired read event...");
        return;
    }

    //LOG_DEBUG("net", "on_read: sock: %d, internal: %d, idx = %d, mask = %d", ctx->peer.sock, ctx->internal, idx, WORKER_THREAD_SIZE_MASK);
    s_pending_read[ctx->worker_idx].push(ctx);
}

static void handle_read(context_t *ctx)
{
    int size = CHUNK_SIZE_L;
    int sock = ctx->peer.sock;
    chunk_queue chunks;

    char buf[CHUNK_SIZE_L];
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
                context_stop(ctx);
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
            LOG_INFO("net", "%s is closed by peer.", ctx->peer.info);
            context_stop(ctx);
            return;
        }

        chunk_t *c = chunk_init(buf, size);
        assert(c->wr_offset > 0);

        // append received chunk
        chunks.push_back(c);
        if (chunks.size() > CHUNK_MAX_NUM && ctx->internal == false) {
            chunk_queue::iterator itr = chunks.begin();

            for (itr = chunks.begin(); itr != chunks.end(); ++itr) {
                chunk_fini(*itr);
            }
            context_stop(ctx);
            LOG_ERROR("net", "%s <%d> chunk num over range, force closed.",
                    ctx->peer.info, chunks.size());
            return;
        }
    }
    if (chunks.size() >= CHUNK_MAX_NUM / 2) {
        LOG_WARN("net", "%s chunk num over range: <%d>", ctx->peer.info, chunks.size());
    }
    push_recv(ctx, chunks);
}



static void on_write(const epoll_event &evt)
{
    int instance;
    context_t *ctx = (context_t*)(evt.data.ptr);
    instance = (uintptr_t)ctx & 1;
    ctx = (context_t*)((uintptr_t)ctx & (uintptr_t) ~1);
    if (ctx->peer.sock == -1 || ctx->instance != instance) {
        LOG_DEBUG("net", "%s", "expired write event...");
        return;
    }

    if (ctx != NULL && ctx->alive() == 1) {
        blob_t *msg = E_NEW blob_t;
        blob_init(msg, ctx);

        //LOG_DEBUG("net", "on_write: sock: %d, internel: %d, idx = %d, mask = %d", ctx->peer.sock, ctx->internal, idx, WORKER_THREAD_SIZE_MASK);
        s_pending_write[ctx->worker_idx].push(msg);
    }
}

static bool handle_write(context_t *ctx)
{
    int sock = ctx->peer.sock;
    int sum = 0;
    chunk_queue &chunks = ctx->send_data->chunks;

    while (ctx->alive() && !chunks.empty()) {
        chunk_t *c = chunks.front();
        if (c == NULL) continue;

        int rem = c->wr_offset - c->rd_offset;
        while (ctx->alive() && rem > 0) {
            int num = send(sock, c->data + c->rd_offset, rem, 0);
            if (num < 0) {
                if (errno != EINTR && errno != EAGAIN) {
                    LOG_ERROR("net", "%s send FAILED: %s.",
                            ctx->peer.info,
                            strerror(errno));
                } else {
                    if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock, &(ctx->evt))) {
                        LOG_INFO("net", "%s epoll_ctl FAILED: %s.",
                                ctx->peer.info,
                                strerror(errno));
                    }
                }
                goto stat;
            } else {
                rem -= num;
                c->rd_offset += num;
                sum += num;
            }
        }
        chunks.pop_front();
        chunk_fini(c);
        if (sum >= 1024 * 32 && !ctx->internal) {
            if (0 != epoll_ctl(s_epoll, EPOLL_CTL_MOD, ctx->peer.sock, &(ctx->evt))) {
                LOG_INFO("net", "%s epoll_ctl FAILED: %s.",
                        ctx->peer.info,
                        strerror(errno));
            }
            break;
        }
    }

stat:
    ctx->send_data->pending_size -= sum;
    return chunks.empty();
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
