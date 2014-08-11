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

#include <elf/config.h>
#include <elf/mutex.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <sys/epoll.h>
#include <string>

namespace elf {
struct blob_t;

struct recv_message_t {
    std::string name;
    std::string body;
    oid_t peer;
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
/// Start server.
/// @param peer Server id, not used now.
/// @param name Server name.
/// @param ip Listen ip string.
/// @param port Listen port.
/// @return (0).
///
int net_listen(oid_t peer, const std::string &name,
        const std::string &ip, int port);

///
/// Start client.
/// @param peer Peer id.
/// @param name Peer name.
/// @param ip Peer ip string.
/// @param port Peer port.
/// @return (0).
///
int net_connect(oid_t peer, const std::string &name,
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
/// Check if connected.
/// @param peer Peer id.
/// @return true if connected, or false.
///
bool net_connected(oid_t peer);

///
/// Get peer address ip:port.
/// @param[in] peer Peer id.
/// @param[out] str Peer info.
/// @return Peer address info.
///
void net_peer_addr(oid_t peer, char *str);

///
/// Get peer address info.
/// @param[in] peer Peer id.
/// @param[out] str Peer info.
/// @return Peer address info.
///
void net_peer_info(oid_t peer, char *str);

///
/// Error occurred.
/// @param[in] peer Peer id.
/// @return Error occurred times.
///
int net_error(oid_t peer);

///
/// Encode sent message.
/// @param pb Protobuf data.
/// @return Encoded message.
///
blob_t *net_encode(const pb_t &pb);

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
void net_send(const id_map &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const id_imap &peers, const pb_t &pb);

///
/// Multicast to given peers.
/// @param[in] peers Peers list.
/// @param[in] pb Message.
///
void net_send(const obj_map_id &peers, const pb_t &pb);
} // namespace elf

#endif /* !ELF_NET_NET_H */

