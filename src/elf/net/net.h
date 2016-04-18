/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file net/net.h
 * @author Fox(yulefox@gmail.com)
 * @date 2014-01-27
 * @brief Network module.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_NET_NET_H
#define ELF_NET_NET_H

#include <elf/cipher.h>
#include <elf/config.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <sys/epoll.h>
#include <string>

namespace elf {
typedef void (*encrypt_func)(char* buf, int len);
struct blob_t;
struct context_t;

struct recv_message_t {
    std::string name;
    std::string body;
    oid_t peer;
    context_t *ctx;
    pb_t *pb;
};

///
/// Initialize the network module.
/// @return (0).
///
int net_init(void);

///
/// Release the network module.
/// @return (0).
///
int net_fini(void);

///
/// Add encrypt/decrypt function.
/// @param encry Encrypt function.
/// @param decry Decrypt function.
///
void net_encrypt(encrypt_func encry, encrypt_func decry);

///
/// Start server.
/// @param name Server name.
/// @param ip Listen ip string.
/// @param port Listen port.
/// @return (0).
///
int net_listen(const std::string &name, const std::string &ip, int port);

///
/// Start client.
/// @param idx Application index.
/// @param peer Peer id.
/// @param name Peer name.
/// @param ip Peer ip string.
/// @param port Peer port.
/// @return (0).
///
int net_connect(int idx, oid_t peer, const std::string &name,
        const std::string &ip, int port);

///
/// Disconnect peer and release associated context.
/// @param peer Peer id.
///
void net_close(oid_t peer);

///
/// Process all received messages.
/// @return (0).
///
int net_proc(void);

///
/// Output statistics info.
/// @return (0).
///
void net_stat(void);

///
/// Output statistics info of given peer.
/// @param[in] peer Peer id.
///
void net_peer_stat(oid_t peer);

///
/// Get peer address ip.
/// @param[in] ctx Context.
/// @return Peer info.
///
const char *net_peer_ip(const context_t *ctx);

///
/// Get peer address info.
/// @param[in] ctx Context.
/// @return Peer info.
///
const char *net_peer_info(const context_t *ctx);

///
/// Error occurred.
/// @param[in] ctx Context.
/// @return Error occurred times.
///
int net_error(context_t *ctx);

///
/// Encode sent message.
/// @param pb Protobuf data.
/// @return Encoded message.
///
blob_t *net_encode(const pb_t &pb);
//blob_t *net_encode(oid_t peer, const pb_t &pb);

///
/// Decode and release received message.
/// @param msg Receive message data.
/// @return true if decoded done, or false.
///
bool net_decode(recv_message_t *msg);

///
/// Send message.
/// @param peer Peer id.
/// @param msg Encoded message data.
/// @return (0).
///
int net_send(oid_t peer, blob_t *msg);

///
/// Unicast to given peer.
/// @param[in] peer Peer id.
/// @param[in] pb Message.
///
void net_send(oid_t peer, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const id_set &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const obj_map_id &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const pb_map_id &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const id_limap &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const id_ilmap &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const obj_map_id &peers, const pb_t &pb);

void net_rawsend(oid_t peer, const std::string &name, const std::string &body);

void net_cipher_set(oid_t peer, cipher_t *encipher, cipher_t *decipher);

void net_register_raw(const std::string &name);

void net_internal_set(oid_t peer, bool flag);

bool net_internal(const context_t &ctx);
} // namespace elf

#endif /* !ELF_NET_NET_H */

