/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file net/message.h
 * @author Fox(yulefox@gmail.com)
 * @date 2014-01-27
 * @brief Network message module.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_NET_MESSAGE_H
#define ELF_NET_MESSAGE_H

#include <elf/config.h>
#include <elf/memory.h>
#include <elf/object.h>
#include <elf/pb.h>
#include <elf/net/net.h>
#include <map>

namespace elf {
typedef void (*msg_proc)(const recv_message_t &msg);

void message_unregist_all(void);
void message_regist(const std::string &name, pb_new init, msg_proc proc);

void message_handle(recv_message_t *msg);
void rpc_message_handle(recv_message_t *msg);

} // namespace elf

#endif /* !ELF_NET_MESSAGE_H */

