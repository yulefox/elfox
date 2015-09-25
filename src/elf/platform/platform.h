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
#include <cJSON/cJSON.h>

namespace elf {

typedef int (*auth_cb)(int plat_type, const std::string &channel,
        int code, cJSON *resp, void *args);

enum platform_type {
    PLAT_INVALID    = -1,
    PLAT_PP         = 1,
    PLAT_I4         = 2,
    PLAT_LJ         = 3,
    PLAT_1SDK       = 4,
    PLAT_UC         = 5,
};

enum platform_error {
    PLATFORM_OK                 = 0,
    PLATFORM_TYPE_ERROR         = -1,
    PLATFORM_SETTING_ERROR      = -2,
    PLATFORM_PARAM_ERROR        = -3,
    PLATFORM_RESPONSE_FAILED    = -4,
    PLATFORM_USER_NOT_LOGININ   = -5,
    PLATFORM_USER_NOT_EXIEST    = -6,
    PLATFORM_UNKOWN_ERROR       = -7,
};

int platform_init();
int platform_fini();
int platform_load(int type, const char *proto);
int platform_auth(int plat_type, const char *data, auth_cb cb, void *args);
int platform_proc();


} // namespace elf


#endif /* !_ELF_PLATFORM_H */

