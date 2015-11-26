/*
 * Author: youngtrips
 * Created Time:  Mon 26 Oct 2015 04:52:53 PM CST
 * File Name: base64.h
 * Description: 
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_BASE64_H
#define ELF_BASE64_H

#include <elf/config.h>
#include <string>

namespace elf {
    std::string base64_encode(const char *input, int length, bool with_new_line);
    std::string base64_decode(char *input, int length, bool with_new_line);
} // namespace elf

#endif /* !ELF_BASE64_H */

