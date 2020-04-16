#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_NET_RPC_H
#define ELF_NET_RPC_H

#include <elf/cipher.h>
#include <elf/config.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

namespace elf {
namespace rpc {

struct MetaData {
    std::string key;
    std::string val;

    MetaData(const std::string &_key, const std::string &_val) : key(_key), val(_val) {}
    MetaData(const std::string &_key, int _val) : key(_key), val(std::to_string(_val)) {}
    MetaData(const std::string &_key, int64_t _val) : key(_key), val(std::to_string(_val)) {}
    ~MetaData() {}
};

int init();

int open(int id, const std::string &name, oid_t peer, const std::string &ip, int port,
        const std::vector<MetaData> &metaList,
        const std::string &caFile = "",
        const std::string &privKeyFile = "",
        const std::string &certFile = "");
int send(const std::string &name, const pb_t &pb, void *ctx = NULL);
int send(oid_t peer, const pb_t &pb, void *ctx = NULL);
int proc(void);

} // namespace rpc
} // namespace elf

#endif /* !ELF_NET_RPC_H */

