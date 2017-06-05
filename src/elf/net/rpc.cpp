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
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include <elf/net/proto/service.pb.h>
#include <elf/net/proto/service.grpc.pb.h>

namespace elf {
namespace rpc {


struct RpcSession {
    std::deque<proto::Packet*> wque;
    std::mutex mutex;
    std::shared_ptr<grpc::Channel>  channel;
    std::thread *watcher;
    std::thread *reader;
    std::thread *writer;
    std::string name;
    std::string ip;
    int serverId;
    int scope;
    int port;
    oid_t peer;

    ///
    void send(const pb_t &pb) {
        std::string name = pb.GetTypeName();
        std::string payload;
        pb.SerializeToString(&payload);
        proto::Packet *pkt = E_NEW proto::Packet;
        pkt->set_type(name);
        pkt->set_payload(payload);

        std::unique_lock<std::mutex> lock(mutex);
        wque.push_back(pkt);
    }
};



static std::map<oid_t, std::shared_ptr<struct RpcSession> > s_sessions;
static std::map< std::string, oid_t> s_name_ids;
static xqueue<recv_message_t*> s_recv_msgs;
static std::mutex s_lock_id;
static std::mutex s_lock_name;


std::shared_ptr<struct RpcSession> RpcSessionInit(const std::string &name,
        oid_t peer,
        const std::string &ip, int port,
        const std::string &ca,
        const std::string &key,
        const std::string &cert,
        int serverId,
        int scope)
{

    std::shared_ptr<struct RpcSession> s = std::make_shared<struct RpcSession>();

    char raddr[128];
    sprintf(raddr, "%s:%d", ip.c_str(), port);

    grpc::SslCredentialsOptions ssl_opts = {ca, key, cert};
    auto ssl_creds = grpc::SslCredentials(ssl_opts);

    s->wque.clear();
    s->channel = grpc::CreateChannel(raddr, ssl_creds);
    s->watcher = NULL;
    s->reader = NULL;
    s->writer = NULL;
    s->name = name;
    s->serverId = serverId;
    s->scope = scope;
    s->ip = ip;
    s->port = port;
    s->peer = peer;

    std::unique_lock<std::mutex> lock1(s_lock_id);
    std::unique_lock<std::mutex> lock2(s_lock_name);
    s_sessions.insert(make_pair(peer, s));
    s_name_ids.insert(make_pair(name, peer));

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
        std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> > stream)
{
    while (s->channel->GetState(false) == GRPC_CHANNEL_READY) {
        proto::Packet pkt;
        while (stream->Read(&pkt)) {
            recv_message_t *msg = E_NEW recv_message_t;
            msg->name = pkt.type();
            msg->body = pkt.payload();
            msg->peer = s->peer,
            msg->ctx = (elf::context_t*)((void*)s.get());
            msg->pb = NULL;
            msg->rpc = true;
            s_recv_msgs.push(msg);
        }
        usleep(500000);
    }
    LOG_ERROR("net", "rpc reader quit: %lld ", s->peer);
}


static void write_routine (std::shared_ptr<struct RpcSession> s,
        std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> > stream)
{
    while (s->channel->GetState(false) == GRPC_CHANNEL_READY) {
        std::unique_lock<std::mutex> lock(s->mutex);
        if (!s->wque.empty()) {
            proto::Packet *pkt = s->wque.front();
            if (pkt == NULL || stream->Write(*pkt)) {
                s->wque.pop_front();
                E_DELETE pkt;
            }
        }
        lock.unlock();
        usleep(500000);
    }
    LOG_ERROR("net", "rpc writer quit: %lld ", s->peer);
}

static void watch_routine (std::shared_ptr<struct RpcSession> s)
{
    std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> > stream = NULL;
    std::thread *reader = NULL;
    std::thread *writer = NULL;
    grpc::ClientContext *context = NULL;
    for (;;) {
        if (s->channel->GetState(true) == GRPC_CHANNEL_CONNECTING) {
            if (stream) {
                stream->Finish();
            }

            s->channel->WaitForConnected(
                    gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(30, GPR_TIMESPAN)));

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

            std::unique_ptr<proto::GameService::Stub> stub = proto::GameService::NewStub(s->channel);
            if (context == NULL) {
                context = new grpc::ClientContext();

                // metadata
                context->AddMetadata("server_id", std::to_string(s->serverId));
                context->AddMetadata("scope", std::to_string(s->scope));
                context->set_wait_for_ready(false);
            }
            stream = std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> >(stub->Tunnel(context));
            reader = new std::thread(read_routine, s, stream);
            writer = new std::thread(write_routine, s, stream);
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
        message_handle(msg);
        S_DELETE(msg->pb);
        S_DELETE(msg);
    }
    return 0;
}


///
int open(const std::string &name,
        oid_t peer,
        const std::string &ip,
        int port,
        const std::string &caFile,
        const std::string &privKeyFile,
        const std::string &certFile,
        int serverId,
        int scope)
{
    std::string ca_cert;
    std::string key;
    std::string cert;

    if (caFile != "" && privKeyFile != "" && certFile != "") {
        readCfg(caFile, ca_cert);
        readCfg(privKeyFile, key);
        readCfg(certFile, cert);
    }

    std::shared_ptr<struct RpcSession> s = RpcSessionInit(name, peer, ip, port, ca_cert, key, cert, serverId, scope);
    RpcSessionStart(s);
    return 0;
}

int send(const std::string &name, const pb_t &pb)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(name);
    if (s == NULL) {
        return -1;
    }

    s->send(pb);
    return 0;
}

int send(oid_t peer, const pb_t &pb)
{
    std::shared_ptr<struct RpcSession> s = RpcSessionFind(peer);
    if (s == NULL) {
        return -1;
    }

    s->send(pb);
    return 0;
}

int proc(void)
{
    return Proc();
}

} // namespace rpc
} // namespace elf
