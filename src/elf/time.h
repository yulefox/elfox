/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file time.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-01-10
 * @brief The minimum accuracy is millisecond.
 * Do we need high-resolution performance counter?
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_TIME_H
#define ELF_TIME_H

#include <elf/config.h>
#include <stdint.h>
#include <time.h>

#if defined(ELF_PLATFORM_LINUX)
#   include <sys/time.h>
#endif /* ELF_PLATFORM_LINUX */

namespace elf {
typedef uint64_t time64_t;

#if defined(ELF_PLATFORM_WIN32)
#   if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
struct timeval
{
    long tv_sec;
    long tv_usec;
};
#   endif
#endif /* ELF_PLATFORM_WIN32 */

///
/// time.
/// @return Current time.
///
time_t time_s(void);

///
/// gettimeofday.
/// @return Current time.
///
time64_t time_ms(void);

time64_t time_diff(time64_t end, time64_t start);
} // namespace elf

#endif /* !ELF_TIME_H */

