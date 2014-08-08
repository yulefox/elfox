/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * msg://www.yulefox.com/
 */

/**
 * @file net/msg.h
 * @author Fox(yulefox@gmail.com)
 * @date 2014-08-07
 * @brief Message queue IPC.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_NET_MSG_H
#define ELF_NET_MSG_H

#include <elf/config.h>

namespace elf {
///
/// Initialize the MSG module.
/// @return (0).
///
int msg_init(void);

///
/// Release the MSG module.
/// @return (0).
///
int msg_fini(void);

///
/// MSG request in json.
/// @param url URL.
/// @param json json.
/// @param func MSG response callback handle.
/// @param args Callback arguments.
/// @return (0).
///
int msg_json(const char *url, char *json,
        msg_response func, void *args);
} // namespace elf

#endif /* !ELF_NET_MSG_H */

