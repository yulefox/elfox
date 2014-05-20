/*
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file log.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-08-29
 * @brief Encapsulate log4cplus(1.1.0RC1).
 *
 * The configuration file must be found in <I>CONFIG/log.conf<I>.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_LOG_H
#define ELF_LOG_H

#include <elf/config.h>

#define ELF_USE_LOG4CPLUS
#if defined(ELF_USE_LOG4CPLUS)
#   if defined(ELF_PLATFORM_WIN32)
#       if defined(ELF_DEBUG)
#           pragma comment(lib, "log4cplusD.lib")
#       else
#           pragma comment(lib, "log4cplus.lib")
#       endif
#   endif
#   include <log4cplus/logger.h>
#   include <log4cplus/configurator.h>
#   include <log4cplus/loggingmacros.h>
#else
#   include <stdio.h>
#   include <stdlib.h>
#endif /* ELF_USE_LOG4CPLUS */

namespace elf {
/**
 * @return 0
 */
ELF_API int log_init(void);

/**
 * @return 0
 */
ELF_API int log_fini(void);

#if defined(ELF_PLATFORM_WIN32)
#   define WRITE_RAW(file, fmt, ...) do { \
        char buf[MAX_LOG_LENGTH]; \
        snprintf(buf, MAX_LOG_LENGTH, fmt, ##__VA_ARGS__); \
        fwrite(buf, strlen(buf), 1, file); \
    } while (0)
#endif /* ELF_PLATFORM_WIN32 */

#if defined(ELF_PLATFORM_LINUX)
#   define WRITE_RAW(file, fmt, args...) do { \
        char buf[MAX_LOG_LENGTH]; \
        snprintf(buf, MAX_LOG_LENGTH, fmt, ##args); \
        fwrite(buf, strlen(buf), 1, file); \
    } while (0)
#endif /* ELF_PLATFORM_LINUX */

#ifdef ELF_USE_LOG4CPLUS
#   define LOG_RAW LOG4CPLUS_MACRO_FMT_BODY
#else
#   if defined(ELF_PLATFORM_WIN32)
#       define LOG_RAW(logger, level, fmt, ...) \
            WRITE_RAW(stdout, fmt, ##__VA_ARGS__)
#   endif /* ELF_PLATFORM_WIN32 */
#   if define(ELF_PLATFORM_LINUX)
#       define LOG_RAW(logger, level, fmt, args...) \
            WRITE_RAW(stdout, fmt, ##args)
#   endif /* ELF_PLATFORM_LINUX */
#endif /* ELF_USE_LOG4CPLUS */

#if defined(ELF_PLATFORM_WIN32)
#   define LOG_FMT_COLOR(color, logger, level, fmt, ...) do { \
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); \
        CONSOLE_SCREEN_BUFFER_INFO csbi; \
        GetConsoleScreenBufferInfo(handle, &csbi); \
        SetConsoleTextAttribute(handle, color); \
        LOG_RAW(logger, level, fmt, ##__VA_ARGS__); \
        SetConsoleTextAttribute(handle, csbi.wAttributes); \
    } while (0)
#endif /* ELF_PLATFORM_WIN32 */

#if defined(ELF_PLATFORM_LINUX)
#   define LOG_FMT_COLOR(color, logger, level, fmt, args...) do { \
        printf(color); \
        LOG_RAW(logger, level, fmt, ##args); \
        printf(COLOR_NORMAL); \
    } while (0)
#endif /* ELF_PLATFORM_LINUX */

#if defined(ELF_PLATFORM_WIN32)
#   define COLOR_TRACE      0x07
#   define COLOR_DEBUG      0x0b
#   define COLOR_INFO       0x0a
#   define COLOR_WARN       0x0e
#   define COLOR_ERROR      0x0c
#   define COLOR_FATAL      0x0c
#   define COLOR_NORMAL     0x07
#endif /* ELF_PLATFORM_WIN32 */

#if defined(ELF_PLATFORM_LINUX)
#   define COLOR_TRACE      "\033[0m"
#   define COLOR_DEBUG      "\033[0m"
#   define COLOR_INFO       "\033[37m"
#   define COLOR_WARN       "\033[33m"
#   define COLOR_ERROR      "\033[35m"
#   define COLOR_FATAL      "\033[31m"
#   define COLOR_NORMAL     "\033[0m"
#endif /* ELF_PLATFORM_LINUX */

#if defined(ELF_PLATFORM_WIN32)
#   define LOG_TRACE(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_TRACE, logger, TRACE_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   define LOG_DEBUG(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_DEBUG, logger, DEBUG_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   define LOG_INFO(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_INFO, logger, INFO_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   define LOG_WARN(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_WARN, logger, WARN_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   define LOG_ERROR(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_ERROR, logger, ERROR_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   define LOG_FATAL(logger, fmt, ...) \
        LOG_FMT_COLOR(COLOR_FATAL, logger, FATAL_LOG_LEVEL, fmt, ##__VA_ARGS__)

#   ifdef ELF_DEBUG
#       define LOG_TEST(fmt, ...) LOG_INFO("test", fmt, ##__VA_ARGS__)
#       define LOG_NOIMPL() LOG_WARN("noimpl", __FUNCTION__)
#   else
#       define LOG_TEST(fmt, ...) LOG_TRACE("test", fmt, ##__VA_ARGS__)
#       define LOG_NOIMPL()
#   endif /* ELF_DEBUG  */
#endif /* ELF_PLATFORM_WIN32 */

#if defined(ELF_PLATFORM_LINUX)
#   define LOG_TRACE(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_TRACE, logger, TRACE_LOG_LEVEL, fmt, ##args)

#   define LOG_DEBUG(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_DEBUG, logger, DEBUG_LOG_LEVEL, fmt, ##args)

#   define LOG_INFO(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_INFO, logger, INFO_LOG_LEVEL, fmt, ##args)

#   define LOG_WARN(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_WARN, logger, WARN_LOG_LEVEL, fmt, ##args)

#   define LOG_ERROR(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_ERROR, logger, ERROR_LOG_LEVEL, fmt, ##args)

#   define LOG_FATAL(logger, fmt, args...) \
        LOG_FMT_COLOR(COLOR_FATAL, logger, FATAL_LOG_LEVEL, fmt, ##args)

#   ifdef ELF_DEBUG
#       define LOG_TEST(fmt, args...) LOG_INFO("test", fmt, ##args)
#       define LOG_NOIMPL() LOG_WARN("noimpl", __FUNCTION__)
#   else
#       define LOG_TEST(fmt, args...) LOG_TRACE("test", fmt, ##args)
#       define LOG_NOIMPL()
#   endif /* ELF_DEBUG  */
#endif /* ELF_PLATFORM_LINUX */
} // namespace elf

#endif /* !ELF_LOG_H */

