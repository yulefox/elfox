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

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include <elf/net/proto/service.pb.h>
#include <elf/net/proto/service.grpc.pb.h>

namespace elf {
namespace rpc {


void read(const std::string& filename, std::string& data)
{
    std::ifstream file (filename.c_str(), std::ios::in);
	if (file.is_open()) {
        std::stringstream ss;
		ss << file.rdbuf();

        file.close ();
		data = ss.str();
	}
}

class Client {
public:
    static std::shared_ptr<Client> Create(const std::string &name,
            oid_t peer,
            const std::string &ip,
            int port,
            const std::string &ca,
            const std::string &key,
            const std::string &cert,
            const std::string &serverId) {
        std::shared_ptr<Client> client = std::make_shared<Client>(name, peer, ip, port, ca, key, cert, serverId);
        s_clients.insert(make_pair(peer, client));
        s_name_ids.insert(make_pair(name, peer));
        return client;
    }

    static std::shared_ptr<Client> Find(oid_t peer) {
        std::map<oid_t, std::shared_ptr<Client> >::iterator itr = s_clients.find(peer);
        if (itr == s_clients.end()) {
            return NULL;
        }
        return itr->second;
    }

    static std::shared_ptr<Client> Find(const std::string &name) {
        std::map< std::string, oid_t >::iterator itr = s_name_ids.find(name);
        if (itr == s_name_ids.end()) {
            return NULL;
        }
        return Find(itr->second);
    }

    Client(const std::string &name, oid_t peer, const std::string &ip, int port,
            const std::string &ca, const std::string &key, const std::string &cert,
            const std::string &serverId) 
        : name_(name)
        , peer_(peer)
        , ip_(ip)
        , port_(port)
        , server_id_(serverId) {
        char raddr[128];
        sprintf(raddr, "%s:%d", ip.c_str(), port);

        grpc::SslCredentialsOptions ssl_opts = {ca, key, cert};
        auto ssl_creds = grpc::SslCredentials(ssl_opts);

        channel_ = grpc::CreateChannel(raddr, ssl_creds);
    }

    ~Client() {}

    void Start() {
        stub_ = proto::GameService::NewStub(channel_);
        ctx_.AddMetadata("server_id", server_id_);

        stream_ = std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> >(stub_->Tunnel(&ctx_));
        writer_ = std::thread([this]() {
            for (;;) {
                std::deque<proto::Packet*>  packets;
                std::deque<proto::Packet*>::iterator itr;
                this->wque_.swap(packets);
                for (itr = packets.begin(); itr != packets.end(); ++itr) {
                    proto::Packet *pkt = *itr;
                    this->stream_->Write(*pkt);
                    E_DELETE pkt;
                }
                usleep(500000);
            }
            LOG_ERROR("net", "rpc writer quit: %lld ", this->peer_);
        });
        reader_ = std::thread([this]() {
            for (;;) {
                int st = this->channel_->GetState(true);
                LOG_ERROR("net", "rpc connection state: %d", st);
                if (st == GRPC_CHANNEL_READY) {
                    proto::Packet pkt;
                    while (this->stream_->Read(&pkt)) {
                        LOG_ERROR("net", "rpc<%lld><%s> type<%s>", this->peer_, this->name_.c_str(), pkt.type().c_str());
                        recv_message_t *msg = E_NEW recv_message_t;
                        msg->name = pkt.type();
                        msg->body = pkt.payload();
                        msg->peer = this->peer_;
                        msg->ctx = (elf::context_t*)((void*)this);
                        msg->pb = NULL;
                        msg->rpc = true;
                        s_recv_msgs.push(msg);
                    }
                }
                usleep(500000);
            }
            LOG_ERROR("net", "rpc reader quit: %lld ", this->peer_);
        });
    }

    void Send(const pb_t &pb) {
        std::string name = pb.GetTypeName();
        std::string payload;
        pb.SerializeToString(&payload);
        proto::Packet *pkt = E_NEW proto::Packet;
        pkt->set_type(name);
        pkt->set_payload(payload);
        wque_.push(pkt);
    }

    static int Proc() {
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

private:
    static std::map<oid_t, std::shared_ptr<Client> > s_clients;
    static std::map< std::string, oid_t> s_name_ids;
    static xqueue<recv_message_t*> s_recv_msgs;

private:
    xqueue<proto::Packet*> wque_;
    std::shared_ptr<grpc::ClientReaderWriter<proto::Packet, proto::Packet> > stream_;
    std::shared_ptr<grpc::Channel>  channel_;
    std::thread reader_;
    std::thread writer_;
    grpc::ClientContext ctx_;
    std::unique_ptr<proto::GameService::Stub> stub_;

    std::string name_;
    oid_t peer_;
    std::string ip_;
    int port_;
    std::string server_id_;
};


//
std::map<oid_t, std::shared_ptr<Client> > Client::s_clients;
std::map< std::string, oid_t> Client::s_name_ids;
xqueue<recv_message_t*> Client::s_recv_msgs;

///
int open(const std::string &name, oid_t peer, const std::string &ip, int port,
        const std::string &caFile,
        const std::string &privKeyFile,
        const std::string &certFile,
        const std::string &serverId)
{
    std::string ca_cert;
    std::string key;
    std::string cert;

    if (caFile != "" && privKeyFile != "" && certFile != "") {
        read(caFile, ca_cert);
        read(privKeyFile, key);
        read(certFile, cert);
    }

    std::shared_ptr<Client> client = Client::Create(name, peer, ip, port, ca_cert, key, cert, serverId);
    client->Start();
    return 0;
}

int send(const std::string &name, const pb_t &pb)
{
    std::shared_ptr<Client> client = Client::Find(name);
    if (client == NULL) {
        return -1;
    }
    client->Send(pb);
    return 0;
}

int send(oid_t peer, const pb_t &pb)
{
    std::shared_ptr<Client> client = Client::Find(peer);
    if (client == NULL) {
        return -1;
    }
    client->Send(pb);
    return 0;
}

int proc(void)
{
    return Client::Proc();
}

} // namespace rpc
} // namespace elf
