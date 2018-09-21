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
MetaData::MetaData(const std::string &_key, const std::string &_val) : key(_key), val(_val)
    {}

MetaData::MetaData(const std::string &_key, int _val) {
    key = _key;
    val = std::to_string(_val);
}

MetaData::MetaData(const std::string &_key, int64_t _val) {
    key = _key;
    val = std::to_string(_val);
}

MetaData::~MetaData() {}

struct RpcSession {
    std::deque<pb::Packet*> wque;
    std::mutex mutex;
    std::shared_ptr<grpc::Channel>  channel;
    std::thread *watcher;
    std::thread *reader;
    std::thread *writer;
    std::string name;
    std::string ip;
    grpc::SslCredentialsOptions ssl_opts;
    bool enable_ssl;
    int port;
    int id;
    std::vector<MetaData> metaList;
    oid_t peer;

    ///
    void send(const pb_t &pb, void *ctx) {
        std::string name = pb.GetTypeName();
        std::string payload;
        pb.SerializeToString(&payload);

        pb::Packet *pkt = E_NEW pb::Packet;
        pb::Peer *peer = pkt->mutable_peer();
        pb::Peer *to = static_cast<pb::Peer*>(ctx);
        if (to == NULL) {
            peer->set_name("gs");
            peer->add_peers(id);
        } else {
            peer->CopyFrom(*to);
        }
        pkt->set_type(name);
        pkt->set_payload(payload);

        std::unique_lock<std::mutex> lock(mutex);
        wque.push_back(pkt);
    }

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
    }

};



static std::map<oid_t, std::shared_ptr<struct RpcSession> > s_sessions;
static std::map< std::string, oid_t> s_name_ids;
static xqueue<recv_message_t*> s_recv_msgs;
static std::mutex s_lock_id;
static std::mutex s_lock_name;


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
    s->wque.clear();
    s->watcher = NULL;
    s->reader = NULL;
    s->writer = NULL;
    s->name = name;
    s->ip = ip;
    s->port = port;
    s->peer = peer;
    s->metaList = metaList;

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


static void read_routine (std::shared_ptr<struct RpcSession> s,
        std::shared_ptr<grpc::ClientReaderWriter<pb::Packet, pb::Packet> > stream)
{
    LOG_INFO("net", "rpc reader start: %lld ", s->peer);
    while (s->channel->GetState(false) == GRPC_CHANNEL_READY) {
        pb::Packet pkt;
        while (stream->Read(&pkt)) {
            // message source
            pb::Peer *from = E_NEW pb::Peer;
            from->CopyFrom(pkt.peer());

            recv_message_t *msg = E_NEW recv_message_t;
            msg->name = pkt.type();
            msg->body = pkt.payload();
            msg->peer = s->peer,
            msg->ctx = (elf::context_t*)((void*)s.get());
            msg->pb = NULL;
            msg->rpc_ctx = (void*)from;
            s_recv_msgs.push(msg);
        }
        usleep(500000);
    }
    LOG_ERROR("net", "rpc reader quit: %lld ", s->peer);
}


static void write_routine (std::shared_ptr<struct RpcSession> s,
        std::shared_ptr<grpc::ClientReaderWriter<pb::Packet, pb::Packet> > stream)
{
    LOG_INFO("net", "rpc writer start: %lld ", s->peer);
    std::deque<pb::Packet*> pending;
    while (s->channel->GetState(false) == GRPC_CHANNEL_READY) {
        // get cached pkts
        std::unique_lock<std::mutex> lock(s->mutex);
        while(!s->wque.empty()) {
            pb::Packet *pkt = s->wque.front();
            pending.push_back(pkt);
            s->wque.pop_front();
        }
        lock.unlock();

        // try to send
        if (!pending.empty()) {
            pb::Packet *pkt = pending.front();
            if (pkt == NULL || stream->Write(*pkt)) {
                pending.pop_front();
                E_DELETE pkt;
            }
        }
        usleep(500000);
    }
    LOG_ERROR("net", "rpc writer quit: %lld ", s->peer);
}

static void watch_routine (std::shared_ptr<struct RpcSession> s)
{
    std::shared_ptr<grpc::ClientReaderWriter<pb::Packet, pb::Packet> > stream = NULL;
    std::thread *reader = NULL;
    std::thread *writer = NULL;
    grpc::ClientContext *context = NULL;
    for (;;) {
        int stat = s->channel->GetState(true);
        if (stat == GRPC_CHANNEL_TRANSIENT_FAILURE) {
            //s->open();
            LOG_INFO("net", "rpc reopen<%s><%s:%d>", s->name.c_str(), s->ip.c_str(), s->port);
        } else if (stat == GRPC_CHANNEL_CONNECTING) {
            LOG_INFO("net", "wait rpc <%s><%s:%d> to be connected", s->name.c_str(), s->ip.c_str(), s->port);
            while (s->channel->GetState(false) == GRPC_CHANNEL_CONNECTING) {
                gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(5, GPR_TIMESPAN));
                if (!s->channel->WaitForConnected(deadline)) {
                    LOG_INFO("net", "rpc <%s><%s:%d> do not be connected", s->name.c_str(), s->ip.c_str(), s->port);
                } else {
                    if (context != NULL) {
                        delete context;
                        context = NULL;
                    }

                    if (reader != NULL) {
                        reader->join();
                        delete reader;
                    }

                    if (writer != NULL) {
                        writer->join();
                        delete writer;
                    }

                    std::unique_ptr<pb::DBus::Stub> stub = pb::DBus::NewStub(s->channel);
                    if (context == NULL) {
                        context = new grpc::ClientContext();

                        // metadata
                        for (size_t i = 0; i < s->metaList.size(); i++) {
                            context->AddMetadata(s->metaList[i].key, s->metaList[i].val);
                            LOG_INFO("net", "add metadata %s %s", s->metaList[i].key.c_str(), s->metaList[i].val.c_str());
                        }
                        //context->set_wait_for_ready(false);
                    }
                    context->set_wait_for_ready(true);

                    stream = std::shared_ptr<grpc::ClientReaderWriter<pb::Packet, pb::Packet> >(stub->Stream(context));
                    reader = new std::thread(read_routine, s, stream);
                    writer = new std::thread(write_routine, s, stream);
                    LOG_INFO("net", "rpc <%s><%s:%d> established.", s->name.c_str(), s->ip.c_str(), s->port);
                }
            }
        }
        usleep(500000);
    }
}

static void RpcSessionStart(std::shared_ptr<struct RpcSession> s)
{
    s->watcher = new std::thread(watch_routine, s);
}

static void RpcSessionStop(std::shared_ptr<struct RpcSession> s)
{
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

    // disable signal
    grpc_use_signal(-1);
    std::shared_ptr<struct RpcSession> s = RpcSessionInit(id, name, peer, ip, port, ca_cert, key, cert, metaList);
    RpcSessionStart(s);
    return 0;
}

int send(const std::string &name, const pb_t &pb, void *ctx)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(name);
    if (s == NULL) {
        return -1;
    }

    s->send(pb, ctx);
    return 0;
}

int send(oid_t peer, const pb_t &pb, void *ctx)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(peer);
    if (s == NULL) {
        return -1;
    }

    s->send(pb, ctx);
    return 0;
}

int proc(void)
{
    return Proc();
}

} // namespace rpc
} // namespace elf
