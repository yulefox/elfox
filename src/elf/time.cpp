/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/time.h>

namespace elf {
#define EPOCHFILETIME 116444736000000000LL

/**
 * Do same thing as Linux.
 */
#ifdef ELF_PLATFORM_WIN32
static int gettimeofday(struct timeval *tp, void *tzp);
#endif

time_t time_s(void)
{
    time_t tm;

    time(&tm);
    return tm;
}

time64_t time_ms(void)
{
#if defined(ELF_PLATFORM_WIN32)
    return GetTickCount();
#else
    struct timeval tv;

    memset(&tv, 0, sizeof(tv));
    gettimeofday(&tv, NULL);
    return ((time64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

time64_t time_diff(time64_t end, time64_t start)
{
    return (end - start);
}

#ifdef ELF_PLATFORM_WIN32
static int gettimeofday(struct timeval *tp, void *tzp)
{
    FILETIME ft;
    LARGE_INTEGER li;           /* union defination  */
    long long t;
    static int tzflag;

    if (tp) {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t = li.QuadPart;        /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tp->tv_sec  = (long)(t / 1000000LL);
        tp->tv_usec = (long)(t % 1000000LL);
    }

    /** @menuHandler timezone? */
    /*
    if (tzp)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        (struct timezone *)tzp->tz_minuteswest = _timezone / 60;
        (struct timezone *)tzp->tz_dsttime = _daylight;
    }
    */

    return 0;
}
#endif /* ELF_PLATFORM_WIN32 */
} // namespace elf

