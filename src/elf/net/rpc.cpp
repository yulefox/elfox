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

#include <grpc/grpc.h>
#include <grpc/grpc_posix.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include <elf/net/proto/dbus.pb.h>
#include <elf/net/proto/dbus.grpc.pb.h>

namespace elf {
namespace rpc {

static std::map< std::string, oid_t> s_name_ids;
static xqueue<recv_message_t*> s_recv_msgs;
static std::mutex s_lock_id;
static std::mutex s_lock_name;
static const int64_t KEEPALIVE_TIMEOUT = 2000; // 2 seconds


class DBusClient {
public:
    explicit DBusClient(int id, oid_t peer, const std::vector<MetaData> &metaList,
            std::shared_ptr<grpc::Channel> channel) {
            
        metaList_ = metaList;
        channel_ = channel;
        stub_ = pb::DBus::NewStub(channel);
        id_ = id;
        peer_ = peer;

        //Init();
    }

    void Init() {
        LOG_INFO("net", "%s", "start initialize stream...");
        AsyncClientCall *call = new AsyncClientCall;
        call->oper = AsyncClientCall::OperType::INIT;

        // metadata
        for (size_t i = 0; i < metaList_.size(); i++) {
            call->context.AddMetadata(metaList_[i].key, metaList_[i].val);
            LOG_INFO("net", "add metadata %s %s", metaList_[i].key.c_str(), metaList_[i].val.c_str());
        }
        //call->context.set_wait_for_ready(true);
        stream_ = stub_->AsyncStream(&call->context, &cq_, call);
    }

    void PushSend(const pb_t &pb, void *ctx) {
        std::string name = pb.GetTypeName();
        std::string payload;
        pb.SerializeToString(&payload);


        pb::Packet *pkt = E_NEW pb::Packet;
        pb::Peer *peer = pkt->mutable_peer();
        pb::Peer *to = static_cast<pb::Peer*>(ctx);
        if (to == NULL) {
            peer->set_name("gs");
            peer->add_peers(id_);
        } else {
            peer->CopyFrom(*to);
        }
        pkt->set_type(name);
        pkt->set_payload(payload);

        std::unique_lock<std::mutex> lock(mutex_);
        wque_.push_back(pkt);

        LOG_INFO("net", "push send: %s", name.c_str());

        // start request write
        if (wque_.size() == 1) {
            LOG_INFO("net", "%s", "launch to send...");
            nextWrite();
        }
    }

    static void WorkerRoutine(DBusClient *clt) {
        clt->Loop();
    }

    void Loop() {
        int prev_st = GRPC_CHANNEL_IDLE;
        for (;;) {
            void* got_tag = NULL;
            bool ok = false;

            gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(1, GPR_TIMESPAN));
            grpc::CompletionQueue::NextStatus st = cq_.AsyncNext(&got_tag, &ok, deadline);
            if (st == grpc::CompletionQueue::SHUTDOWN) {
                LOG_ERROR("net", "%s", "grpc::CompletionQueue has been shutdown...");
                break;
            } else if (st == grpc::CompletionQueue::TIMEOUT) {
                int curr_st = channel_->GetState(true);
                if ((prev_st == GRPC_CHANNEL_IDLE ||
                     prev_st == GRPC_CHANNEL_CONNECTING ||
                     prev_st == GRPC_CHANNEL_TRANSIENT_FAILURE) && curr_st == GRPC_CHANNEL_READY) {
                    Init();
                }
                LOG_INFO("net", "grpcloop...channel stat: prev(%d) curr(%d)", prev_st, curr_st);
                prev_st = curr_st;
            } else if (st == grpc::CompletionQueue::GOT_EVENT) {
                if (ok) {
                    AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);
                    switch (call->oper) {
                    case AsyncClientCall::OperType::INIT:
                        onInit(call);
                        break;
                    case AsyncClientCall::OperType::READ:
                        onRead(call);
                        break;
                    case AsyncClientCall::OperType::WRITE:
                        onWrite(call);
                        break;
                    default:
                        LOG_INFO("net", "%s", "get an unknown type operation");
                        break;
                    }
                    //delete call;
                }
            }
        }
    }

private:
    struct AsyncClientCall {
        enum OperType { INIT = 0x1, READ = 0x2, WRITE = 0x4};
        OperType oper;
        grpc::ClientContext context;
        pb::Packet pkt;
    };

    void nextRead() {
        AsyncClientCall *call = new AsyncClientCall;
        call->oper = AsyncClientCall::OperType::READ;
        stream_->Read(&call->pkt, call);
    }

    void nextWrite() {
        pb::Packet *pkt = wque_.front();
        AsyncClientCall *call = new AsyncClientCall;
        call->oper = AsyncClientCall::OperType::WRITE;
        stream_->Write(*pkt, (void*)call);
    }

    void onInit(AsyncClientCall *call) {
        nextRead();

        LOG_INFO("net", "%s", "grpc on inited...");
    }

    void onRead(AsyncClientCall *call) {
        LOG_INFO("net", "%s", "grpc on read...");

        pb::Packet *pkt = &(call->pkt);

        pb::Peer *from = E_NEW pb::Peer;
        from->CopyFrom(pkt->peer());

        recv_message_t *msg = E_NEW recv_message_t;
        msg->name = pkt->type();
        msg->body = pkt->payload();
        msg->peer = peer_;
        msg->pb = NULL;
        msg->rpc_ctx = (void*)from;
        msg->ctx = (elf::context_t*)((void*)this);

        //msg->ctx = (elf::context_t*)((void*)s.get());
        //
        s_recv_msgs.push(msg);

        LOG_INFO("net", "recv msg: %s", msg->name.c_str());

        //
        nextRead();
    }

    void onWrite(AsyncClientCall *call) {
        LOG_INFO("net", "%s", "grpc on write...");
        std::unique_lock<std::mutex> lock(mutex_);
        wque_.pop_front();

        // keep request to send
        if (!wque_.empty()) {
            nextWrite();
        }
    }

private:
    std::unique_ptr<grpc::ClientAsyncReaderWriter<pb::Packet, pb::Packet> > stream_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<pb::DBus::Stub> stub_;
    std::deque<pb::Packet*> wque_;
    std::mutex mutex_;
    std::vector<MetaData> metaList_;
    grpc::ClientContext context_;
    grpc::CompletionQueue cq_;
    oid_t peer_;
    int id_;
};

struct RpcSession {
    std::shared_ptr<grpc::Channel>  channel;
    std::string name;
    std::string ip;
    grpc::SslCredentialsOptions ssl_opts;
    bool enable_ssl;
    int port;
    int id;
    std::vector<MetaData> metaList;
    oid_t peer;
    std::thread *worker;

    DBusClient *client;

    void open()
    {
        char raddr[128];
        sprintf(raddr, "%s:%d", ip.c_str(), port);

        if (enable_ssl) {
            auto ssl_creds = grpc::SslCredentials(ssl_opts);
            channel = grpc::CreateChannel(raddr, ssl_creds);
        } else {
            channel = grpc::CreateChannel(raddr, grpc::InsecureChannelCredentials());
        }

        client = new DBusClient(id, peer, metaList, channel);
        worker = new std::thread(&DBusClient::WorkerRoutine, client);
    }

    void close() {
        delete client;
        worker->join();
        delete worker;
    }
};

//
static std::map<oid_t, std::shared_ptr<struct RpcSession> > s_sessions;

std::shared_ptr<struct RpcSession> RpcSessionInit(int id, const std::string &name,
        oid_t peer,
        const std::string &ip, int port,
        const std::string &ca,
        const std::string &key,
        const std::string &cert,
        const std::vector<MetaData> &metaList)
{

    std::shared_ptr<struct RpcSession> s = std::make_shared<struct RpcSession>();

        if (ca != "" && key != "" && cert != "") {
        s->enable_ssl = true;
        s->ssl_opts = {ca, key, cert};
    }

    s->id = id;
    s->name = name;
    s->ip = ip;
    s->port = port;
    s->peer = peer;
    s->metaList = metaList;
    s->client = NULL;
    s->worker = NULL;

    std::unique_lock<std::mutex> lock1(s_lock_id);
    std::unique_lock<std::mutex> lock2(s_lock_name);
    s_sessions.insert(make_pair(peer, s));
    s_name_ids.insert(make_pair(name, peer));

    s->open();

    return s;
}


std::shared_ptr<struct RpcSession> RpcSessionFind(oid_t peer)
{
    std::unique_lock<std::mutex> lock(s_lock_id);

    std::map<oid_t, std::shared_ptr<struct RpcSession> >::iterator itr = s_sessions.find(peer);
    if (itr == s_sessions.end()) {
        return NULL;
    }
    return itr->second;
}

std::shared_ptr<struct RpcSession> RpcSessionFind(const std::string &name)
{
    std::unique_lock<std::mutex> lock(s_lock_name);

    std::map< std::string, oid_t >::iterator itr = s_name_ids.find(name);
    if (itr == s_name_ids.end()) {
        return NULL;
    }
    return RpcSessionFind(itr->second);
}

static void readCfg(const std::string& filename, std::string& data)
{
    std::ifstream file (filename.c_str(), std::ios::in);
	if (file.is_open()) {
        std::stringstream ss;
		ss << file.rdbuf();

        file.close ();
		data = ss.str();
	}
}

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

    if (caFile != "" && privKeyFile != "" && certFile != "") {
        readCfg(caFile, ca_cert);
        readCfg(privKeyFile, key);
        readCfg(certFile, cert);
    }

    std::shared_ptr<struct RpcSession> s = RpcSessionInit(id, name, peer, ip, port, ca_cert, key, cert, metaList);
    //RpcSessionStart(s);
    return 0;
}

int send(const std::string &name, const pb_t &pb, void *ctx)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(name);
    if (s == NULL || s->client == NULL) {
        return -1;
    }

    s->client->PushSend(pb, ctx);
    return 0;
}

int send(oid_t peer, const pb_t &pb, void *ctx)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(peer);
    if (s == NULL || s->client == NULL) {
        return -1;
    }

    s->client->PushSend(pb, ctx);
    return 0;
}

int proc(void)
{
    return Proc();
}



} // namespace rpc
} // namespace elf
