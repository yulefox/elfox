/**
 * @file macosx.h
 * @date 2014-03-24
 * @author Fox (yulefox at gmail.com)
 */

#ifndef ELF_CONFIG_MACOSX_H
#define ELF_CONFIG_MACOSX_H

#pragma once

#if defined(__MACOS__)
#   define ELF_PLATFORM_MACOSX
#endif

#define ELF_HAVE_SYS_TYPES_H
#define ELF_HAVE_SYS_LOCKING_H
#define ELF_HAVE_FCNTL_H
#define ELF_HAVE_IO_H
#define ELF_HAVE_STDIO_H
#define ELF_HAVE_WCHAR_H
#define ELF_HAVE_STDARG_H
#define ELF_HAVE_STDLIB_H
#define ELF_HAVE_ERRNO_H
#define ELF_HAVE_SYS_STAT_H
#define ELF_HAVE_TIME_H
#define ELF_HAVE_STDLIB_H

#if defined(__GNUC__)
#  undef ELF_INLINES_ARE_EXPORTED
#  define ELF_HAVE_FUNCTION_MACRO
#  define ELF_HAVE_GNU_VARIADIC_MACROS
#  define ELF_HAVE_C99_VARIADIC_MACROS
#  if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#    define ELF_HAVE_PRETTY_FUNCTION_MACRO
#  endif
#endif

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH size_t(_PC_PATH_MAX)

#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

#endif /* !ELF_CONFIG_MACOSX_H */

