/*
 * Author: youngtrips
 * Created Time:  Mon 26 Oct 2015 04:52:53 PM CST
 * File Name: base64.h
 * Description: 
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_RSA_H
#define ELF_RSA_H

#include <elf/config.h>
#include <string>

namespace elf {
    int RSAVerify(std::string ctx, std::string sign, std::string pubkey);
} // namespace elf

#endif /* !ELF_RSA_H */

