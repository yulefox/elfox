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


class DBusClient {
public:
    explicit DBusClient(int id, oid_t peer, const std::vector<MetaData> &metaList,
            std::shared_ptr<grpc::Channel> channel) {
            
        metaList_ = metaList;
        channel_ = channel;
        stub_ = pb::DBus::NewStub(channel);
        id_ = id;
        peer_ = peer;
        revent_ = E_NEW AsyncClientCall(AsyncClientCall::OperType::READ);
        wevent_ = E_NEW AsyncClientCall(AsyncClientCall::OperType::WRITE);
        context_ = NULL;
        running_ = 1;
        ready_ = 0;
    }

    virtual ~DBusClient() {
        setRunning(0);
        cq_.Shutdown();
        std::unique_lock<std::mutex> lock(mutex_);
        while (!wque_.empty()) {
            pb::Packet *pkt = wque_.front();
            if (pkt != NULL) {
                E_DELETE pkt;
            }
            wque_.pop_front();
        }
        lock.unlock();

        E_DELETE context_;
        E_DELETE revent_;
        E_DELETE wevent_;
    }

    void Init() {
        LOG_INFO("rpc", "%s", "start initialize stream...");
        setReady(0);
        revent_->Init(AsyncClientCall::OperType::INIT);
        if (context_ != NULL) {
            context_->TryCancel();
            E_DELETE context_;
        }
        // metadata
        context_ = E_NEW grpc::ClientContext;
        for (size_t i = 0; i < metaList_.size(); i++) {
            context_->AddMetadata(metaList_[i].key, metaList_[i].val);
            LOG_INFO("rpc", "init clientcontext, add metadata %s %s", metaList_[i].key.c_str(), metaList_[i].val.c_str());
        }

        //
        stream_ = stub_->AsyncStream(context_, &cq_, revent_);
        LOG_INFO("rpc", "%s", "start initialize stream...OK");
    }

    void PushSend(const pb_t &pb, void *ctx) {
        if (!isRunning()) {
            LOG_INFO("rpc", "%s", "asynclient has been shutdown.");
            return;
        }
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

        LOG_INFO("rpc", "push send: %s", name.c_str());

        // start request write
        if (isReady() && wque_.size() == 1) {
            LOG_INFO("rpc", "%s", "launch to send...");
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

            gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_millis(100, GPR_TIMESPAN));
            grpc::CompletionQueue::NextStatus st = cq_.AsyncNext(&got_tag, &ok, deadline);
            if (st == grpc::CompletionQueue::SHUTDOWN) {
                LOG_ERROR("rpc", "%s", "grpc::CompletionQueue has been shutdown...");
                break;
            } else if (st == grpc::CompletionQueue::TIMEOUT) {
                int curr_st = channel_->GetState(true);
                if (prev_st != curr_st && curr_st == GRPC_CHANNEL_CONNECTING) {
                    LOG_INFO("rpc", "%s", "try to reconnect to grpc server.");
                }
                if ((prev_st == GRPC_CHANNEL_IDLE ||
                     prev_st == GRPC_CHANNEL_CONNECTING ||
                     prev_st == GRPC_CHANNEL_TRANSIENT_FAILURE) && curr_st == GRPC_CHANNEL_READY) {
                    LOG_INFO("rpc", "%s", "grpc reconnected.");
                    Init();
                }
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
                        LOG_INFO("rpc", "%s", "get an unknown type operation");
                        break;
                    }
                }
            }
        }
    }

private:
    struct AsyncClientCall {
        enum OperType { INIT = 0x1, READ = 0x2, WRITE = 0x4};
        OperType oper;
        pb::Packet pkt;

        AsyncClientCall() {
            pkt.Clear();
        }

        AsyncClientCall(OperType op) {
            oper = op;
            pkt.Clear();
        }

        ~AsyncClientCall() {}

        void Init(OperType op) {
            oper = op;
            pkt.Clear();
        }
    };

    void nextRead() {
        revent_->Init(AsyncClientCall::OperType::READ);
        stream_->Read(&revent_->pkt, revent_);
    }

    void nextWrite() {
        pb::Packet *pkt = wque_.front();
        if (pkt != NULL) {
            wevent_->Init(AsyncClientCall::OperType::WRITE);
            stream_->Write(*pkt, wevent_);
        } else {
            wque_.pop_front();
            LOG_ERROR("rpc", "%s", "got an nullptr from wque, do drop it.");
        }
    }

    void onInit(AsyncClientCall *call) {
        nextRead();
   
        // launch to send
        std::unique_lock<std::mutex> lock(mutex_);
        setReady(1);
        if (!wque_.empty()) {
            nextWrite();
        }
        LOG_INFO("rpc", "%s", "grpc on inited...try to launch to read && write.");
    }

    void onRead(AsyncClientCall *call) {
        LOG_INFO("rpc", "%s", "grpc on read...");

        pb::Packet *pkt = &(call->pkt);

        pb::Peer *from = E_NEW pb::Peer;
        from->CopyFrom(pkt->peer());

        recv_message_t *msg = E_NEW recv_message_t;
        msg->name = pkt->type();
        msg->body = pkt->payload();
        msg->peer = peer_;
        msg->pb = NULL;
        msg->rpc_ctx = (void*)from;
        msg->ctx = (elf::context_t*)((void*)call);

        //
        s_recv_msgs.push(msg);

        LOG_INFO("rpc", "recv msg: %s", msg->name.c_str());

        //
        nextRead();
    }

    void onWrite(AsyncClientCall *call) {
        LOG_INFO("rpc", "%s", "grpc on write...");
        std::unique_lock<std::mutex> lock(mutex_);
        wque_.pop_front();

        // try to send
        if (!wque_.empty()) {
            nextWrite();
        }
    }

private:

    void setRunning(int flag) {
        __sync_val_compare_and_swap(&running_, (flag ^ 1), flag);
    }

    bool isRunning() {
        return __sync_val_compare_and_swap(&running_, 1, 1) == 1;
    }

    void setReady(int flag) {
        __sync_val_compare_and_swap(&ready_, (flag ^ 1), flag);
    }

    bool isReady() {
        return __sync_val_compare_and_swap(&ready_, 1, 1) == 1;
    }

private:
    std::unique_ptr<grpc::ClientAsyncReaderWriter<pb::Packet, pb::Packet> > stream_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<pb::DBus::Stub> stub_;
    std::deque<pb::Packet*> wque_;
    std::mutex mutex_;
    std::vector<MetaData> metaList_;
    grpc::CompletionQueue cq_;
    grpc::ClientContext *context_;
    AsyncClientCall *revent_;
    AsyncClientCall *wevent_;
    int running_;
    int ready_;
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

        grpc::ChannelArguments args;
        args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 32 * 1024 * 1024); // 32M
        args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, 32 * 1024 * 1024); // 32M
        args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 3000);
        args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5000); 
        args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        args.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, 1000); // The minimum time between subsequent connection attempts, in ms.
        args.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, 3000); // The maximum time between subsequent connection attempts, in ms.
        args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 500); // The time between the first and second connection attempts, in ms.
        if (enable_ssl) {
            auto ssl_creds = grpc::SslCredentials(ssl_opts);
            channel = grpc::CreateCustomChannel(raddr, ssl_creds, args);
        } else {
            channel = grpc::CreateCustomChannel(raddr, grpc::InsecureChannelCredentials(), args);
        }

        client = E_NEW DBusClient(id, peer, metaList, channel);
        worker = E_NEW std::thread(&DBusClient::WorkerRoutine, client);
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
