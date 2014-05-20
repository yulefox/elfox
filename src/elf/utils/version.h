/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file utils/version.h
 * @author Fox (yulefox at gmail.com)
 * @date 2014-02-21
 * @brief Version parsing.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_UTILS_VERSION_H
#define ELF_UTILS_VERSION_H

#include <elf/config.h>
#include <string>

namespace elf {
struct version_t {
    int major;
    int minor;
    int patch;
    int build;
};

///
/// Parse string version.
/// @param[out] ver Version data.
/// @param[in] str Version string.
/// @return 0 if done, or -1.
///
int version_parse(version_t *ver, const std::string &str);

///
/// Compare versions.
/// @param[in] left Version string.
/// @param[in] right Version string.
/// @return 0 if left == right, -1 if left < right, 1 if left > right.
///
int version_compare(const std::string &left, const std::string &right);

///
/// Compare versions.
/// @param[in] left Version data.
/// @param[in] right Version data.
/// @return 0 if left == right, -1 if left < right, 1 if left > right.
///
int version_compare(const version_t &left, const version_t &right);
} // namespace elf

#endif /* !ELF_UTILS_VERSION_H */

