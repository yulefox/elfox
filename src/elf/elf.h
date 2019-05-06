/**
 * Copyright (C) 2011-2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file elf.h
 * @date 2011-08-29
 * @author Fox (yulefox at gmail.com)
 * 
 */

#ifndef ELF_ELF_H
#define ELF_ELF_H

#define ELF_USE_CP
#define ELF_USE_DB
#define ELF_USE_LOG
#define ELF_USE_MEMORY
#define ELF_USE_OID
#define ELF_USE_PB
#define ELF_USE_SINGLETON

#if defined ELF_USE_ALL
#   define ELF_USE_OS
#   define ELF_USE_EVENT
#   define ELF_USE_HTTP
#   define ELF_USE_INPUT
#   define ELF_USE_JSON
#   define ELF_USE_LOCK
#   define ELF_USE_MATCHMAKING
#   define ELF_USE_MD5
#   define ELF_USE_NET
#   define ELF_USE_PC
#   define ELF_USE_PIKE
#   define ELF_USE_PLATFORM
#   define ELF_USE_RAND
#   define ELF_USE_RPC
#   define ELF_USE_SCRIPT
#   define ELF_USE_SHM_ALLOC
#   define ELF_USE_SINGLETON
#   define ELF_USE_SKIPLIST
#   define ELF_USE_THREAD
#   define ELF_USE_TIME
#   define ELF_USE_TIMER
#   define ELF_USE_UTILS
#   define ELF_USE_VERSION
#   define ELF_USE_BTDC
#endif /* ELF_USE_MIN */

#define ELF_INIT(mod) \
    do { \
        elf::mod##_init(); \
    } while (0)

#define ELF_FINI(mod) \
    do { \
        elf::mod##_fini(); \
    } while (0)

#include <elf/config.h>

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#if defined(ELF_USE_CP)
#   include <elf/cp.h>
#endif /* ELF_USE_CP */

#if defined(ELF_USE_DB)
#   include <elf/db/db.h>
#endif /* ELF_USE_DB */

#if defined(ELF_USE_EVENT)
#   include <elf/event.h>
#endif /* ELF_USE_EVENT */

#if defined(ELF_USE_HTTP)
#   include <elf/net/http.h>
#endif /* ELF_USE_HTTP */

#if defined(ELF_USE_JSON)
#   include <elf/json.h>
#endif /* ELF_USE_JSON */

#if defined(ELF_USE_LCOK)
#   include <elf/lock.h>
#endif /* ELF_USE_LOCK */

#if defined(ELF_USE_LOG)
#   include <elf/log.h>
#endif /* ELF_USE_LOG */

#if defined(ELF_USE_MD5)
#   include <elf/md5.h>
#endif /* ELF_USE_MD5 */

#if defined(ELF_USE_MEMORY)
#   include <elf/memory.h>
#endif /* ELF_USE_MEMORY */

#if defined(ELF_USE_MATCHMAKING)
#   include <elf/matchmaking.h>
#endif /* ELF_USE_MATCHMAKING */

#if defined(ELF_USE_NET)
#   include <elf/net/net.h>
#   include <elf/net/message.h>
#endif /* ELF_USE_NET */

#if defined(ELF_USE_PC)
#   include <elf/pc.h>
#endif /* ELF_USE_PC */

#if defined(ELF_USE_PIKE)
#   include <elf/pike.h>
#endif /* ELF_USE_PIKE */

#if defined(ELF_USE_PLATFORM)
#   include <elf/platform/platform.h>
#endif /* ELF_USE_PLATFORM */

#if defined(ELF_USE_RAND)
#   include <elf/rand.h>
#endif /* ELF_USE_RAND */

#if defined(ELF_USE_RPC)
#   include <elf/net/rpc.h>
#endif /* ELF_USE_RPC */

#if defined(ELF_USE_OID)
#   include <elf/oid.h>
#endif /* ELF_USE_OID */

#if defined(ELF_USE_OS)
#   include <elf/os.h>
#endif /* ELF_USE_OS */

#if defined(ELF_USE_PB)
#   include <elf/pb.h>
#endif /* ELF_USE_PB */

#if defined(ELF_USE_SCRIPT)
#   include <elf/script/script.h>
#endif /* ELF_USE_SCRIPT */

#if defined(ELF_USE_SHM_ALLOC)
#   include <elf/shm_alloc.h>
#endif /* ELF_USE_SHM_ALLOC */

#if defined(ELF_USE_SINGLETON)
#   include <elf/singleton.h>
#endif /* ELF_USE_SINGLETON */

#if defined(ELF_USE_SKIPLIST)
#   include <elf/skiplist.h>
#endif /* ELF_USE_SKIPLIST */

#if defined(ELF_USE_THREAD)
#   include <elf/thread.h>
#endif /* ELF_USE_THREAD */

#if defined(ELF_USE_TIME)
#   include <elf/time.h>
#endif /* ELF_USE_TIME */

#if defined(ELF_USE_TIMER)
#   include <elf/timer.h>
#endif /* ELF_USE_TIMER */

#if defined(ELF_USE_UTILS)
#   include <elf/utils/version.h>
#endif /* ELF_USE_UTILS */

#if defined(ELF_USE_VERSION)
#   include <elf/version.h>
#endif /* ELF_USE_VERSION */

#if defined(ELF_USE_BTDC)
#   include <elf/btdc/btdc.h>
#endif 

#endif /* !ELF_ELF_H */
