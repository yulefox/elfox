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

#ifndef ELF_DIR_H
#define ELF_DIR_H

#include <elf/config.h>

namespace elf {
///
/// Make directory.
/// @param[in] dir Directory to be made.
/// @return See mkdir(2).
///
int dir_make(const char *dir);
} // namespace elf

#endif /* !ELF_DIR_H */

