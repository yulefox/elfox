/**
 * Copyright (C) 2011-2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file   config.h
 * @date   2011-08-31
 * @author Fox (yulefox at gmail.com)
 */

#ifndef ELF_CONFIG_H
#define ELF_CONFIG_H

#if defined(_WIN32)
#   include <elf/config/win32.h>
#elif (defined(__MWERKS__) && defined(__MACOS__))
#   include <elf/config/macosx.h>
#else
#   include <elf/config/linux.h>
#endif

#if defined(_MSC_VER)                                                   \
    || (defined(__COMO__) && __COMO_VERSION__ >= 400) /* ??? */         \
    || (defined(__DMC__) && __DMC__ >= 0x700) /* ??? */                 \
    || (defined(__clang__) && __clang_major__ >= 3)                     \
    || (defined(__GNUC__) && (__GNUC__ >= 4                             \
        || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)))
#   undef ELF_HAVE_PRAGMA_ONCE
#   define ELF_HAVE_PRAGMA_ONCE
#   pragma once
#endif

#ifdef _DEBUG
#   define ELF_DEBUG
#else
#   define ELF_RELEASE
#endif

/* @{ */
#ifdef __cplusplus
#   define BEGIN_C_DECLS     extern "C" {
#   define END_C_DECLS       }
#else
typedef enum { false, true } bool;
#   define BEGIN_C_DECLS
#   define END_C_DECLS
#endif /* __cplusplus */
/* @} */

/* @{ */
#if defined(ELF_STATIC) && defined(ELF_DYNAMIC)
#  error ELF_STATIC and ELF_DYNAMIC cannot be defined both.
#endif

#if defined(ELF_PLATFORM_WIN32) && defined(ELF_DYNAMIC)
#   ifdef ELF_EXPORTS
#       define ELF_API      __declspec(dllexport)
#       define ELF_DATA     __declspec(dllexport)
#   else
#       define  ELF_API     __declspec(dllimport)
#       define  ELF_DATA    __declspec(dllimport)
#   endif
#else
#   define ELF_API          extern
#   define ELF_DATA
#endif
/* @} */

#define ELF_ENTRY           ELF_DATA
#define ELF_INL             inline
#define ELF_ASSERT          assert
#define ELF_UNUSED_ARG

#define MODULE_IMPORT_SWITCH    do { \
        static bool once = true; \
        if (!once) return -1; \
        once = false; \
    } while (0)

#define ELF_MAX_LINE 256
#endif /* !ELF_CONFIG_H */

