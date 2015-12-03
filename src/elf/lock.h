/**
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file lock.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-07-09
 *
 * Locks: mutex/spinlock.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_MUTEX_H
#define ELF_MUTEX_H

#include <elf/config.h>

#if defined(ELF_PLATFORM_WIN32)
#else
#   include <pthread.h>
#endif /* ELF_PLATFORM_WIN32 */

namespace elf {
#if defined(ELF_PLATFORM_WIN32)
typedef CRITICAL_SECTION mutex_t;
typedef CRITICAL_SECTION spin_t;
#else
typedef pthread_mutex_t mutex_t;
typedef pthread_spinlock_t spin_t;
#endif /* ELF_PLATFORM_WIN32 */

void mutex_init(mutex_t *m);

void mutex_fini(mutex_t *m);

void mutex_lock(mutex_t *m);

void mutex_unlock(mutex_t *m);

void spin_init(spin_t *m);

void spin_fini(spin_t *m);

void spin_lock(spin_t *m);

void spin_unlock(spin_t *m);

class lock {
public:
    lock(mutex_t *m) :
        m_lock(m)
    {
        mutex_lock(m_lock);
    }
    ~lock(void) {
        mutex_unlock(m_lock);
    }
private:
    mutex_t *m_lock;
};
} // namespace elf

#endif /* !ELF_MUTEX_H */

