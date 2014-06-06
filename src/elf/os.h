/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file dir.h
 * @author Fox (yulefox at gmail.com)
 * @date 2014-06-06
 * @brief Directory operation.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_OS_H
#define ELF_OS_H

#include <elf/config.h>

namespace elf {
///
/// Make directory.
/// @param[in] dir Directory to be made.
/// @return See mkdir(2).
///
int os_mkdir(const char *dir);

///
/// Change or add an environment variable.
/// @param[in] val "name=value".
/// @return See putenv(3).
///
int os_putenv(char *val);

///
/// Get an environment variable.
/// @param[in] name Environment variable.
/// @return value of the environment variable, see getenv(3).
///
const char *os_getenv(const char *name);

///
/// Get an environment variable.
/// @param[in] name Environment variable.
/// @return int value of the environment variable, see getenv(3).
///
int os_getenv_int(const char *name);
} // namespace elf

#endif /* !ELF_OS_H */

