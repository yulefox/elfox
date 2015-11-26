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
typedef std::list< oid_t > id_list;
typedef std::set< oid_t > id_set;
typedef std::map< oid_t, int > id_limap;
typedef std::map< int, oid_t > id_ilmap;
typedef std::map< oid_t, oid_t > id_llmap;
typedef std::map< int, id_set * > id_ismap;
typedef std::map< oid_t, id_set * > id_lsmap;
typedef std::map< int, id_llmap * > id_illmap;
typedef std::map< oid_t, id_llmap * > id_lllmap;
typedef bool (*callback)(void *args);

const oid_t OID_NIL = 0;

struct callback_t {
    oid_t tid; // trigger id
    int evt; // trigger event
    int targ_a; // trigger argument a
    int targ_b; // trigger argument b
    oid_t oid; // owner id
    oid_t lid; // listener id
    int ltype; // listner type
    int larg; // listener argument
    callback func; // callback function
};

///
/// Generate OID.
/// @return Generated OID.
///
oid_t oid_gen(void);

///
/// Insert into `id_ismap`.
/// @param[in out] mm `id_ismap`.
/// @param[in] idx `id_ismap:key`.
/// @param[in] id `id_ismap:element`.
///
void oid_ismap_add(id_ismap &ism, int idx, oid_t id);

///
/// Remove from `id_ismap`.
/// @param[in out] mm `id_ismap`.
/// @param[in] idx `id_ismap:key`.
/// @param[in] id `id_ismap:element`.
///
void oid_ismap_del(id_ismap &ism, int idx, oid_t id);

///
/// Insert into `id_illmap`.
/// @param[in out] mm `id_illmap`.
/// @param[in] idx `id_illmap:key`.
/// @param[in] k `id_llmap:key`.
/// @param[in] v `id_llmap:value`.
///
void oid_illmap_add(id_illmap &illm, int idx, oid_t k, oid_t v);

///
/// Remove from `id_illmap`.
/// @param[in out] mm `id_illmap`.
/// @param[in] idx `id_illmap:key`.
/// @param[in] k `id_llmap:key`.
///
void oid_illmap_del(id_illmap &illm, int idx, oid_t k);
} // namespace elf

#endif /* !ELF_OID_H */

