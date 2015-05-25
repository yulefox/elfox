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
#include <list>
#include <map>
#include <set>

namespace elf {
typedef int64_t oid_t;
typedef std::list<oid_t> id_list;
typedef std::set<oid_t> id_set;
typedef std::map<oid_t, int> id_map;
typedef std::map<int, oid_t> id_imap;
typedef std::map<oid_t, oid_t> id_pmap;
typedef std::map<oid_t, id_set *> id_psmap;
typedef std::map<int, id_pmap *> id_ipmap;
typedef std::map<oid_t, id_pmap *> id_ppmap;
typedef bool (*callback)(void *args);

const oid_t OID_NIL = 0;

struct callback_t {
    oid_t tid; // trigger id
    int evt; // trigger event
    int targ_a; // trigger argument a
    int targ_b; // trigger argument b
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

///
/// Insert into `id_ipmap`.
/// @param[in out] mm `id_ipmap`.
/// @param[in] idx `id_ipmap:key`.
/// @param[in] id_1 `id_pmap:key`.
/// @param[in] id_2 `id_pmap:value`.
///
void oid_ipmap_add(id_ipmap &mm, int idx, oid_t id_1, oid_t id_2);
} // namespace elf

#endif /* !ELF_OID_H */

