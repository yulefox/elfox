/*
 * Author: youngtrips
 * Created Time:  Mon 06 Jul 2015 08:04:01 PM CST
 * File Name: md5.h
 * Description: 
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_MD5_H
#define ELF_MD5_H

#include <elf/config.h>
#include <string>

namespace elf {
    std::string md5(const unsigned char *d, unsigned long n);
} // namespace elf

#endif /* !ELF_MD5_H */

