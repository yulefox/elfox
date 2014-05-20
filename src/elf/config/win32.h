/**
 * @file win32.h
 * @date 2012-07-18
 * @author Fox (yulefox at gmail.com)
 */

#ifndef ELF_CONFIG_WIN32_H
#define ELF_CONFIG_WIN32_H

#pragma once

#include <assert.h>
#include <winsock2.h>
#include <windows.h>

#if defined(_WINDOWS) || defined(_WIN32) || defined(WIN32)
#   define ELF_PLATFORM_WIN32
#endif

#if (defined(_MSC_VER) && _MSC_VER >= 1400)
// Newer versions of Win32 headers shipped with MinGW have this header, too.
// But at this time it is not possible to recoginze such versions anyhow.
//|| defined(__MINGW32__)
#   define ELF_HAVE_INTRIN_H
#endif

// Time related functions and headers.
#define ELF_HAVE_TIME_H

#if !defined(_WIN32_WCE)
#   define ELF_HAVE_SYS_TIMEB_H
#   define ELF_HAVE_FTIME
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__) 
#   define ELF_HAVE_GMTIME_S
#endif

// Use Winsock on Windows.
#define ELF_USE_WINSOCK

// Enable Win32DebugAppender
#define ELF_HAVE_OUTPUTDEBUGSTRING

// Enable Win32ConsoleAppender.
#define ELF_HAVE_WIN32_CONSOLE

#define ELF_HAVE_SYS_TYPES_H
#define ELF_HAVE_SYS_LOCKING_H
#define ELF_HAVE_FCNTL_H
#define ELF_HAVE_IO_H
#define ELF_HAVE_STDIO_H
#define ELF_HAVE_WCHAR_H
#define ELF_HAVE_STDARG_H
#define ELF_HAVE_STDLIB_H
#if !defined(_WIN32_WCE)
/* Define if you have the ftime function.  */
#   define ELF_HAVE_ERRNO_H
#   define ELF_HAVE_SYS_STAT_H
#endif
#define ELF_HAVE_TIME_H
#define ELF_HAVE_STDLIB_H

// MSVC has both and so does MinGW.
#define ELF_HAVE_VSNPRINTF
#define ELF_HAVE__VSNPRINTF

#if defined(_MSC_VER)
// MS secure versions of vprintf().
#   define ELF_HAVE_VSPRINTF_S
#   define ELF_HAVE_VSWPRINTF_S

// MS secure versions of vfprintf().
#   define ELF_HAVE_VFPRINTF_S
#   define ELF_HAVE_VFWPRINTF_S

// MS secure versions of vsnprintf().
#   define ELF_HAVE_VSNPRINTF_S
#   define ELF_HAVE__VSNPRINTF_S
#   define ELF_HAVE__VSNWPRINTF_S

#   define ELF_INLINES_ARE_EXPORTED

#   if _MSC_VER >= 1400
#       if ! defined(_WIN32_WCE)
#           define ELF_WORKING_LOCALE
#       endif
#       define ELF_HAVE_FUNCTION_MACRO
#       define ELF_HAVE_FUNCSIG_MACRO
#       define ELF_HAVE_C99_VARIADIC_MACROS
#   endif /* _MSC_VER >= 1400 */

#   pragma warning(disable : 4996)
#endif /* _MSC_VER */

#if defined(_WIN32_WCE)
#   define ELF_DLLMAIN_HINSTANCE HANDLE
#   undef ELF_HAVE_NT_EVENT_LOG
#else
#   define ELF_DLLMAIN_HINSTANCE HINSTANCE
#   define ELF_HAVE_NT_EVENT_LOG
#endif

#ifndef ELF_SINGLE_THREADED
#   define ELF_USE_WIN32_THREADS
#endif

#if _WIN32_WINNT + 0 < 0x0600
#   define ELF_POOR_MANS_SHAREDMUTEX
#endif

#if defined(__GNUC__)
#  undef ELF_INLINES_ARE_EXPORTED
#  define ELF_HAVE_FUNCTION_MACRO
#  define ELF_HAVE_GNU_VARIADIC_MACROS
#  define ELF_HAVE_C99_VARIADIC_MACROS
#  if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#    define ELF_HAVE_PRETTY_FUNCTION_MACRO
#  endif
#endif

#define snprintf _snprintf

#define BEGIN_CRASH_DUMP
#define CATCH_CRASH_DUMP

#endif /* !ELF_CONFIG_WIN32_H */
