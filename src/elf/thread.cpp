/*
 * Copyright 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/thread.h>
#include <assert.h>

namespace elf {
thread_t thread_init(thread_func func, void *args)
{
    thread_t tid;

#if defined(ELF_PLATFORM_WIN32)
    tid = (HANDLE)_beginthreadex(NULL, 0, func, args, 0,
        NULL);
    assert(tid != NULL);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_create(&tid, NULL, func, args);
#endif // ELF_PLATFORM_WIN32
    return tid;
}

void thread_fini(thread_t tid)
{
#if defined(ELF_PLATFORM_WIN32)
    WaitForSingleObject(tid, INFINITE);
    CloseHandle(tid);
#elif defined(ELF_PLATFORM_LINUX)
    pthread_join(tid, NULL);
#endif // ELF_PLATFORM_WIN32
}
} // namespace elf

