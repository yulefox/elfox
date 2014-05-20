/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file oid.h
 * @author Fox (yulefox at gmail.com)
 * @date 2013-11-29
 * @brief OID(int 64, long long) generation algorithm.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_OID_H
#define ELF_OID_H

#include <elf/config.h>
#include <stdint.h>
#include <set>

namespace elf {
typedef uint64_t oid_t;
typedef std::set<oid_t> id_set;
typedef void (*callback)(void *args);

const oid_t OID_NIL = 0;

struct callback_t {
    oid_t tid; // trigger id
    int ttype; // trigger type
    int targ; // trigger argument
    oid_t lid; // listener id
    oid_t oid; // object id
    int ltype; // listner type
    int larg; // listener argument
    int ref; // reference
    callback func; // callback function
};

///
/// Generate OID.
/// @return Generated OID.
///
oid_t oid_gen(void);
} // namespace elf

#endif /* !ELF_OID_H */

