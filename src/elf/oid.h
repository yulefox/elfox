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
typedef std::map< oid_t, oid_t > id_llmap;
typedef std::map< int, oid_t > id_ilmap;
typedef std::map< int, id_set * > id_ismap;
typedef std::map< oid_t, id_set * > id_lsmap;

typedef std::map< int, id_llmap * > id_illmap;
typedef std::map< int, id_ismap * > id_iismap;
typedef std::map< int, id_lsmap * > id_ilsmap;

typedef std::map< oid_t, id_ilmap * > id_lilmap;
typedef std::map< oid_t, id_llmap * > id_lllmap;
typedef std::map< oid_t, id_ismap * > id_lismap;
typedef std::map< oid_t, id_lsmap * > id_llsmap;

typedef bool (*callback)(void *args);

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
/// @param[in] key `id_ismap:key`.
/// @param[in] val `id_set:element`.
///
void oid_ismap_add(id_ismap &ism, int key, oid_t val);

///
/// Remove from `id_ismap`.
/// @param[in out] mm `id_ismap`.
/// @param[in] key `id_ismap:key`.
/// @param[in] val `id_set:element`.
///
void oid_ismap_del(id_ismap &ism, int key, oid_t val);

///
/// Insert into `id_lsmap`.
/// @param[in out] mm `id_lsmap`.
/// @param[in] key `id_lsmap:key`.
/// @param[in] val `id_set:element`.
///
void oid_lsmap_add(id_lsmap &lsm, oid_t key, oid_t val);

///
/// Remove from `id_lsmap`.
/// Remove from `id_lsmap`.
/// @param[in] key `id_lsmap:key`.
/// @param[in] val `id_set:element`.
///
void oid_lsmap_del(id_lsmap &lsm, oid_t key, oid_t val);

///
/// Insert into `id_illmap`.
/// @param[in out] illm `id_illmap`.
/// @param[in] key `id_illmap:key`.
/// @param[in] k `id_llmap:key`.
/// @param[in] v `id_llmap:value`.
///
void oid_illmap_add(id_illmap &illm, int key, oid_t k, oid_t v);

///
/// Remove from `id_illmap`.
/// @param[in out] illm `id_illmap`.
/// @param[in] key `id_illmap:key`.
/// @param[in] k `id_llmap:key`.
///
void oid_illmap_del(id_illmap &illm, int key, oid_t k);

///
/// Insert into `id_iismap`.
/// @param[in out] iism `id_iismap`.
/// @param[in] key `id_iismap:key`.
/// @param[in] k `id_ismap:key`.
/// @param[in] v `id_ismap:value`.
///
void oid_iismap_add(id_iismap &iism, int key, int k, oid_t v);

///
/// Remove from `id_iismap`.
/// @param[in out] iism `id_iismap`.
/// @param[in] key `id_ismap:key`.
/// @param[in] k `id_ismap:key`.
/// @param[in] v `id_ismap:value`.
///
void oid_iismap_del(id_iismap &iism, int key, int k, oid_t v);

///
/// Insert into `id_ilsmap`.
/// @param[in out] ilsm `id_ilsmap`.
/// @param[in] key `id_ilsmap:key`.
/// @param[in] k `id_lsmap:key`.
/// @param[in] v `id_lsmap:value`.
///
void oid_ilsmap_add(id_ilsmap &ilsm, int key, oid_t k, oid_t v);

///
/// Remove from `id_ilsmap`.
/// @param[in out] ilsm `id_ilsmap`.
/// @param[in] key `id_lsmap:key`.
/// @param[in] k `id_lsmap:key`.
/// @param[in] v `id_lsmap:value`.
///
void oid_ilsmap_del(id_ilsmap &ilsm, int key, oid_t k, oid_t v);

///
/// Insert into `id_lilmap`.
/// @param[in out] lilm `id_lilmap`.
/// @param[in] key `id_lilmap:key`.
/// @param[in] k `id_llmap:key`.
/// @param[in] v `id_llmap:value`.
///
void oid_lilmap_add(id_lilmap &lilm, oid_t key, int k, oid_t v);

///
/// Remove from `id_lilmap`.
/// @param[in out] lilm `id_lilmap`.
/// @param[in] key `id_lilmap:key`.
/// @param[in] k `id_llmap:key`.
///
void oid_lilmap_del(id_lilmap &lilm, oid_t key, int k);

///
/// Insert into `id_lllmap`.
/// @param[in out] lllm `id_lllmap`.
/// @param[in] key `id_lllmap:key`.
/// @param[in] k `id_llmap:key`.
/// @param[in] v `id_llmap:value`.
///
void oid_lllmap_add(id_lllmap &lllm, oid_t key, oid_t k, oid_t v);

///
/// Remove from `id_lllmap`.
/// @param[in out] lllm `id_lllmap`.
/// @param[in] key `id_lllmap:key`.
/// @param[in] k `id_llmap:key`.
///
void oid_lllmap_del(id_lllmap &lllm, oid_t key, oid_t k);

///
/// Insert into `id_lismap`.
/// @param[in out] lism `id_lismap`.
/// @param[in] key `id_lismap:key`.
/// @param[in] k `id_ismap:key`.
/// @param[in] v `id_ismap:value`.
///
void oid_lismap_add(id_lismap &lism, oid_t key, int k, oid_t v);

///
/// Remove from `id_lismap`.
/// @param[in out] lism `id_lismap`.
/// @param[in] key `id_ismap:key`.
/// @param[in] k `id_ismap:key`.
/// @param[in] v `id_ismap:value`.
///
void oid_lismap_del(id_lismap &lism, oid_t key, int k, oid_t v);

///
/// Insert into `id_llsmap`.
/// @param[in out] llsm `id_llsmap`.
/// @param[in] key `id_llsmap:key`.
/// @param[in] k `id_lsmap:key`.
/// @param[in] v `id_lsmap:value`.
///
void oid_llsmap_add(id_llsmap &llsm, oid_t key, oid_t k, oid_t v);

///
/// Remove from `id_llsmap`.
/// @param[in out] llsm `id_llsmap`.
/// @param[in] key `id_lsmap:key`.
/// @param[in] k `id_lsmap:key`.
/// @param[in] v `id_lsmap:value`.
///
void oid_llsmap_del(id_llsmap &llsm, oid_t key, oid_t k, oid_t v);
} // namespace elf

#endif /* !ELF_OID_H */

