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
    PLAT_HUAWEI     = 6,
    PLAT_VIVO       = 7,
    PLAT_ANZHI      = 8,
    PLAT_QQ         = 9,
    PLAT_WEIXIN     = 10,
    PLAT_APPSTORE   = 11,
    PLAT_MIGU       = 12,
    PLAT_TSIXI      = 13,
    PLAT_FACEBOOK   = 14,
    PLAT_QIANGUI    = 15,
    PLAT_SIFU       = 16,
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

/*
cJSON* platform_get_json(int type);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
int get_channel(cJSON *setting, const char *code, std::string &channel);
*/

int platform_init();
int platform_fini();
int platform_load(int type, const char *proto);
int platform_auth(int plat_type, const char *data, auth_cb cb, void *args);
int platform_proc();


int qg_stat_login(const std::string &userId, int server, int64_t loginTime);
int qg_stat_create(const std::string &userId, int server, int64_t roleId, int64_t createTime);
int qg_stat_online_5m(int server, int total, int64_t time);
int qg_stat_logout(const std::string &userId, int server,
        int64_t roleId, const std::string &roleName,
        int roleLevel, int roleCareer, int roleFightPower,
        int64_t offlineTime);


} // namespace elf


#endif /* !_ELF_PLATFORM_H */

