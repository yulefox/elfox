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
static int gettimeofday(struct timeval *tp, void *tzp)
{
    FILETIME ft;
    LARGE_INTEGER li;           /* union defination  */

    if (tp) {
        long long t;

        GetSystemTimeAsFileTime(&ft);
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t = li.QuadPart;        /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tp->tv_sec  = (long)(t / 1000000LL);
        tp->tv_usec = (long)(t % 1000000LL);
    }
    return 0;
}
#endif /* ELF_PLATFORM_WIN32 */

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

int time_month_days(time_t t)
{
    static const int MONTH_DAY[]= {31,28,31,30,31,30,31,31,30,31,30,31};
    struct tm ltm;

    localtime_r(&t, &ltm);

    int y = ltm.tm_year + 1900;
    int m = ltm.tm_mon;
    int d;

    if (2 == m) {
        d = ((((0 == y % 4) && (0 != y % 100)) || ( 0 == y % 400)) ? 29 : 28);
    } else {
        d = MONTH_DAY[m];
    }
    return d;
}


int get_zoned_time(time_t stamp, int zone, char *cts, size_t size)
{
    if (zone < -11 || zone > 11) {
        return -1;
    }

    time_t curr = stamp;
    if (curr <= 0) {
        curr = time(NULL);
    }
    curr += 3600 * zone;
    struct tm *tm = gmtime(&curr);
    if (tm == NULL) {
        return -1;
    }

    memset(cts, 0, size);
    strftime(cts, size, "%Y-%m-%d %H:%M:%S", tm);
    return 0;
}


} // namespace elf

