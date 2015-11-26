#ifndef __PLATFORM_BASE_H
#define __PLATFORM_BASE_H

#include <elf/elf.h>
#include <elf/platform/platform.h>
#include <cJSON/cJSON.h>

namespace elf {

enum plat_req_type {
    PLAT_REQ_JSON,
};

struct plat_base_req {
    int plat_type;
    int type;
    std::string channel;
    std::string content;
    std::string param;
    void *args;
    cJSON *resp;
    auth_cb cb;

    plat_base_req() : type(0), args(NULL), cb(NULL) {}
    virtual ~plat_base_req() {}

    plat_base_req(int _type, auth_cb _cb, void *_args)
    : type(_type)
    , args(_args)
    , cb(_cb) {}
    virtual bool push_resp(void *ptr, size_t size, bool unquote) = 0;
};

struct plat_base_resp {
    int plat_type;
    int type;
    int code;
    std::string channel;
    auth_cb cb;
    cJSON *resp;
    void *args;

    plat_base_resp()
    : plat_type(0)
    , type(0)
    , code(0)
    , cb(NULL)
    , resp(NULL)
    , args(NULL) {}
};

struct plat_json_req : public plat_base_req {
    plat_json_req() : plat_base_req(PLAT_REQ_JSON, NULL, NULL) {}
    plat_json_req(auth_cb cb, void *args) : plat_base_req(PLAT_REQ_JSON, cb, args) {}
    virtual ~plat_json_req() {}
    virtual bool push_resp(void *ptr, size_t size, bool unquote);
};

// internal funcs
cJSON* platform_get_json(int type);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
int get_channel(cJSON *setting, const char *code, std::string &channel);

int platform_pp_auth(const char *param, auth_cb cb, void *args);
int platform_uc_auth(const char *param, auth_cb cb, void *args);
int platform_i4_auth(const char *param, auth_cb cb, void *args);
int platform_lj_auth(const char *param, auth_cb cb, void *args);
int platform_1sdk_auth(const char *param, auth_cb cb, void *args);
int platform_huawei_auth(const char *param, auth_cb cb, void *args);
int platform_vivo_auth(const char *param, auth_cb cb, void *args);
int platform_anzhi_auth(const char *param, auth_cb cb, void *args);
int platform_qq_auth(const char *param, auth_cb cb, void *args);
int platform_weixin_auth(const char *param, auth_cb cb, void *args);

plat_base_resp* platform_pp_on_auth(const plat_base_req *req);
plat_base_resp* platform_uc_on_auth(const plat_base_req *req);
plat_base_resp* platform_i4_on_auth(const plat_base_req *req);
plat_base_resp* platform_lj_on_auth(const plat_base_req *req);
plat_base_resp* platform_1sdk_on_auth(const plat_base_req *req);
plat_base_resp* platform_huawei_on_auth(const plat_base_req *req);
plat_base_resp* platform_vivo_on_auth(const plat_base_req *req);
plat_base_resp* platform_anzhi_on_auth(const plat_base_req *req);
plat_base_resp* platform_msdk_on_auth(const plat_base_req *req);

} // namespace elf

#endif /* !__PLATFORM_BASE_H */

