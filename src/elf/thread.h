/**
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file thread.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-07-08
 *
 * Thread.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_THREAD_H
#define ELF_THREAD_H

#include <elf/config.h>

#if defined(ELF_PLATFORM_WIN32)
#   include <process.h>
#else
#   include <pthread.h>
#endif /* ELF_PLATFORM_WIN32 */

namespace elf {
#if defined(ELF_PLATFORM_WIN32)
typedef HANDLE thread_t;
typedef unsigned int (__stdcall *thread_func)(void *);
#else
typedef pthread_t thread_t;
typedef void *(*thread_func)(void *);
#endif /* ELF_PLATFORM_WIN32 */

///
/// Thread starts.
/// Return a thread id.
///
thread_t thread_init(thread_func func, void *args);

///
/// Thread exits.
///
void thread_fini(thread_t tid);
} // namespace elf

#endif /* !ELF_THREAD_H */

