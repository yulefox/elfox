#include <elf/elf.h>
#include <elf/lock.h>
#include <elf/net/net.h>
#include <elf/net/rpc.h>
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
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>
#include <future>
#include <jansson.h>

#include <libgohive/bridge.h>
#include <libgohive/libgohive.h>

#include <elf/net/proto/dbus.pb.h>
#include <elf/net/proto/dbus.grpc.pb.h>

namespace elf {
namespace rpc {

struct RpcSession {
    std::string name;
    int id;
    oid_t peer;
};

static std::map< std::string, oid_t> s_name_ids;
static std::map<oid_t, RpcSession* > s_sessions;
static xqueue<recv_message_t*> s_recv_msgs;


static void AddSession(oid_t peer, const std::string &name, int id)
{
    RpcSession *sess = E_NEW RpcSession;
    sess->name = name;
    sess->id = id;
    sess->peer = peer;
    s_sessions[peer] = sess;
    s_name_ids[name] = peer;
}

static const RpcSession *GetSession(oid_t id)
{
    std::map<oid_t, RpcSession*>::iterator itr = s_sessions.find(id);
    if (itr == s_sessions.end()) {
        return NULL;
    }
    return itr->second;
}

static const RpcSession *GetSession(const std::string &name)
{
    std::map< std::string, oid_t >::iterator itr = s_name_ids.find(name);
    if (itr == s_name_ids.end()) {
        return NULL;
    }

    oid_t id = itr->second;;
    return GetSession(id);
}


//

static int Proc()
{
    std::deque<recv_message_t*> msgs;
    std::deque<recv_message_t*>::iterator itr;
    s_recv_msgs.swap(msgs);
    for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
        recv_message_t *msg = *itr;
        rpc_message_handle(msg);

        pb::Peer *peer = static_cast<pb::Peer*>(msg->rpc_ctx);
        S_DELETE(peer);
        S_DELETE(msg->pb);
        S_DELETE(msg);
    }
    return 0;
}


static void on_recv(int size, const void *payload) {
    pb::Packet pkt;
    
    if (!pkt.ParseFromArray(payload, size)) {
        LOG_INFO("rpc", "%s", "unserialize pb::Packet failed.");
        return;
    }

    pb::Peer *from = E_NEW pb::Peer;
    from->CopyFrom(pkt.peer());

    recv_message_t *msg = E_NEW recv_message_t;
    msg->name = pkt.type();
    msg->body = pkt.payload();
    //msg->peer = peer;
    msg->pb = NULL;
    msg->rpc_ctx = (void*)from;
    msg->ctx = (elf::context_t*)((void*)msg);
    msg->is_raw = false;

    //
    s_recv_msgs.push(msg);

    LOG_INFO("rpc", "recv msg: %s", pkt.type().c_str());
}

int init()
{
    HiveInit(on_recv);
    return 0;
}

///
int open(int id, const std::string &name,
        oid_t peer,
        const std::string &ip,
        int port,
        const std::vector<MetaData> &metaList,
        const std::string &caFile,
        const std::string &privKeyFile,
        const std::string &certFile)
{
    std::string ca_cert;
    std::string key;
    std::string cert;

    char addr[256];
    sprintf(addr, "%s:%d", ip.c_str(), port);

    json_t *json = json_object();
    json_t *params = json_object();
    json_t *mds = json_object();
    json_object_set_new(params, "ssl_ca_file", json_string(caFile.c_str()));
    json_object_set_new(params, "ssl_clt_cert_file", json_string(certFile.c_str()));
    json_object_set_new(params, "ssl_clt_key_file", json_string(privKeyFile.c_str()));
    json_object_set_new(json, "addr", json_string(addr));
    json_object_set_new(json, "params", params);

    for (size_t i = 0; i < metaList.size(); i++) {
        MetaData md = metaList[i];
        json_object_set_new(mds, md.key.c_str(), json_string(md.val.c_str()));
    }


    char *ctx = json_dumps(json, 0);
    if (ctx == NULL) {
        LOG_INFO("rpc", "%s", "json_dumps failed");
        return -1;
    }
    LOG_INFO("rpc", "rpc ctx: %s", ctx);

    char *ctx_mds = json_dumps(mds, 0);
    if (ctx_mds == NULL) {
        LOG_INFO("rpc", "%s", "json_dumps failed");
        return -1;
    }
    LOG_INFO("rpc", "rpc mds: %s", ctx_mds);

    char c_name[1024] = {0};
    strcpy(c_name, name.c_str());

    HiveConnect_return res = HiveConnect(peer, c_name, ctx, ctx_mds);
    if (res.r0 != 0) {
        LOG_INFO("rpc", "gohive client init failed: %s", res.r1);
        return -1;
    }

    LOG_INFO("rpc", "%s", "gohive client init success...");
    
    free(ctx);
    free(ctx_mds);
    json_decref(mds);
    json_decref(json);

    AddSession(peer, name, id);
    return 0;
}

static int send(const RpcSession *sess, const pb_t &pb, void *ctx)
{
    if (sess == NULL) {
        LOG_ERROR("rpc", "%s", "invalid rpc session.");
        return -1;
    }
    LOG_INFO("rpc", "try to send...%s %d %lld", sess->name.c_str(), sess->id, sess->peer);

    std::string name = pb.GetTypeName();
    std::string payload;
    pb.SerializeToString(&payload);

    pb::Packet pkt;
    pb::Peer *peer = pkt.mutable_peer();
    pb::Peer *to = static_cast<pb::Peer*>(ctx);
    if (to == NULL) {
        peer->set_name("gs");
        peer->add_peers(sess->id);
    } else {
        peer->CopyFrom(*to);
    }
    pkt.set_type(name);
    pkt.set_payload(payload);

    size_t size = pkt.ByteSizeLong();
    void *buf = malloc(size);
    pkt.SerializeToArray(buf, size);

    HiveSend_return res = HiveSend(sess->peer, size, buf);
    free(buf);
    if (res.r0 != 0) {
        LOG_ERROR("rpc", "gohive send  failed: ", res.r1);
        return -1;
    }
    return 0;
}


int send(const std::string &name, const pb_t &pb, void *ctx)
{
    const RpcSession *sess = GetSession(name);
    if (sess == NULL) {
        LOG_ERROR("rpc", "no found rpc session: %s", name.c_str());
        return -1;
    }
    return send(sess, pb, ctx);
}

int send(oid_t peer, const pb_t &pb, void *ctx)
{
    const RpcSession *sess = GetSession(peer);
    if (sess == NULL) {
        LOG_ERROR("rpc", "no found rpc session: %lld", peer);
    }
    return send(sess, pb, ctx);
}

int proc(void)
{
    return Proc();
}



} // namespace rpc
} // namespace elf
