/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file cp.h
 * @author Fox(yulefox@gmail.com)
 * @date 2013-11-14
 * @brief Configuration file parsing.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_CP_H
#define ELF_CP_H

#include <elf/config.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <string>

namespace elf {
///
/// Load configuration file(synchronous).
/// @param[in] name Configuration protobuf name.
/// @param[in] path Configuration file path.
/// @return Config object if loaded done, or NULL.
///
pb_t *config_load(const std::string &name, const std::string &path);

///
/// Load configuration file(asynchronous).
/// @param[in] path Configuration file path.
/// @param[in] sid Session id.
/// @return true if loaded okey, or false.
///
bool config_load(const std::string &path, oid_t sid);
} // namespace elf

#endif /* !ELF_CP_H */

