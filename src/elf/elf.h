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
#   define ELF_USE_EVENT
//#   define ELF_USE_HTTP
#   define ELF_USE_INPUT
#   define ELF_USE_NET
#   define ELF_USE_PC
#   define ELF_USE_RAND
#   define ELF_USE_SCRIPT
#   define ELF_USE_SINGLETON
#   define ELF_USE_THREAD
#   define ELF_USE_TIME
#   define ELF_USE_TIMER
#   define ELF_USE_UTILS
#   define ELF_USE_VERSION
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
#   include <elf/db.h>
#endif /* ELF_USE_DB */

#if defined(ELF_USE_EVENT)
#   include <elf/event.h>
#endif /* ELF_USE_EVENT */

#if defined(ELF_USE_HTTP)
#   include <elf/net/http.h>
#endif /* ELF_USE_HTTP */

#if defined(ELF_USE_LOG)
#   include <elf/log.h>
#endif /* ELF_USE_LOG */

#if defined(ELF_USE_MEMORY)
#   include <elf/memory.h>
#endif /* ELF_USE_MEMORY */

#if defined(ELF_USE_NET)
#   include <elf/net/net.h>
#   include <elf/net/message.h>
#endif /* ELF_USE_NET */

#if defined(ELF_USE_PC)
#   include <elf/pc.h>
#endif /* ELF_USE_PC */

#if defined(ELF_USE_RAND)
#   include <elf/rand.h>
#endif /* ELF_USE_RAND */

#if defined(ELF_USE_OID)
#   include <elf/oid.h>
#endif /* ELF_USE_OID */

#if defined(ELF_USE_PB)
#   include <elf/pb.h>
#endif /* ELF_USE_PB */

#if defined(ELF_USE_SCRIPT)
#   include <elf/script/script.h>
#endif /* ELF_USE_SCRIPT */

#if defined(ELF_USE_SINGLETON)
#   include <elf/singleton.h>
#endif /* ELF_USE_SINGLETON */

#if defined(ELF_USE_THREAD)
#   include <elf/thread.h>
#   include <elf/mutex.h>
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

#endif /* !ELF_ELF_H */
