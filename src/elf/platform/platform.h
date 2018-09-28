/*
 * Author: youngtrips
 * Created Time:  Mon 06 Jul 2015 01:35:26 PM CST
 * File Name: platform.h
 * Description: 
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef _ELF_PLATFORM_H
#define _ELF_PLATFORM_H

#include <elf/config.h>
#include <elf/pb.h>
#include <openssl/sha.h> 

namespace elf {

enum platform_error {
    PLATFORM_OK                 = 0,
    PLATFORM_SETTING_ERROR      = 1000,
    PLATFORM_TOKEN_INVALID      = 1001,
    PLATFORM_TOKEN_EXPIRED      = 1002,
};

typedef struct platform_user_s {
    int64_t uid;
    std::string sid;
    std::string platform;
    std::string channel;
    std::string sdk;
    std::string account;
    std::string reg_time_s;
    int32_t reg_time;
    int32_t status;
} platform_user_t;

int platform_init(const char *configfile);
int platform_fini();
int platform_auth(const char *token, platform_user_t &puser);

} // namespace elf


#endif /* !_ELF_PLATFORM_H */

