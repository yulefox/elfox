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

typedef int (*auth_cb)(int plat_type, cJSON *resp, void *args);

enum platform_type {
    PLAT_PP,
};

enum platform_error {
    PLATFORM_OK                 = 0,
    PLATFORM_TYPE_ERROR         = -1,
    PLATFORM_SETTING_ERROR      = -2,
    PLATFORM_PARAM_ERROR        = -3,
};

int platform_init();
int platform_fini();
int platform_load(int type, const char *proto);
int platform_auth(int plat_type, const char *data, auth_cb cb, void *args);
int platform_proc();


} // namespace elf


#endif /* !_ELF_PLATFORM_H */

