/*
 * Copyright 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/mutex.h>

namespace elf {
void mutex_init(mutex_t *m)
{
#if defined(ELF_PLATFORM_WIN32)
    InitializeCriticalSection(m);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_mutex_init(m, NULL);
#endif /* ELF_PLATFORM_WIN32 */
}

void mutex_fini(mutex_t *m)
{
#if defined(ELF_PLATFORM_WIN32)
    DeleteCriticalSection(m);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_mutex_destroy(m);
#endif /* ELF_PLATFORM_WIN32 */
}

void mutex_lock(mutex_t *m)
{
#if defined(ELF_PLATFORM_WIN32)
    EnterCriticalSection(m);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_mutex_lock(m);
#endif /* ELF_PLATFORM_WIN32 */
}

void mutex_unlock(mutex_t *m)
{
#if defined(ELF_PLATFORM_WIN32)
    LeaveCriticalSection(m);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_mutex_unlock(m);
#endif /* ELF_PLATFORM_WIN32 */
}
} // namespace elf

