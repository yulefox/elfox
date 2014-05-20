/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/utils/version.h>
#include <string>

namespace elf {
int version_parse(version_t *ver, const std::string &str)
{
    assert(ver);
    memset(ver, 0, sizeof(*ver));

    sscanf(str.c_str(), "%d.%d.%d(%d)",
            &(ver->major), &(ver->minor), &(ver->patch), &(ver->build));
    return 0;
}

int version_compare(const std::string &left, const std::string &right)
{
    version_t ver_l, ver_r;

    version_parse(&ver_l, left);
    version_parse(&ver_r, right);
    return version_compare(ver_l, ver_r);
}

int version_compare(const version_t &left, const version_t &right)
{
    if (left.major > right.major) return 1;
    if (left.major < right.major) return -1;
    if (left.minor > right.minor) return 1;
    if (left.minor < right.minor) return -1;
    if (left.patch > right.patch) return 1;
    if (left.patch < right.patch) return -1;
    return 0;
}
} // namespace elf

