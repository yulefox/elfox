#include <elf/elf.h>
#include <elf/config.h>
#include <elf/net/http.h>
#include <elf/md5.h>
#include <elf/base64.h>
#include <elf/json.h>
#include <elf/time.h>
#include <elf/log.h>
#include <elf/pc.h>
#include <elf/platform/platform.h>
#include <elf/platform/sdks/base.h>
#include <cJSON/cJSON.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>

namespace elf {

plat_base_resp* platform_i4_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *username = cJSON_GetObjectItem(req->resp, "username");
    cJSON *userid = cJSON_GetObjectItem(req->resp, "userid");

    LOG_INFO("platform", "i4 onAuth(): status(%d), username(%s), userid(%d)",
            status->valueint, username->valuestring, userid->valueint);

    int ret = PLATFORM_OK;
    switch (status->valueint) {
    case 0: // success
        ret = PLATFORM_OK;
        break;
    case 1: // token invalid
        ret = PLATFORM_PARAM_ERROR;
        break;
    case 2: // user not exist
        ret = PLATFORM_USER_NOT_EXIEST;
        break;
    case 3: // timeout
        ret = PLATFORM_RESPONSE_FAILED;
        break;
    default:
        ret = PLATFORM_UNKOWN_ERROR;
        break;
    }

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->channel = req->channel;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    return resp;
}

int platform_i4_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_i4_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_I4);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?token=");
    post_url.append(cJSON_GetObjectItem(json, "token")->valuestring);

    cJSON *req_tpl = cJSON_GetObjectItem(setting, "AuthReq");
    if (req_tpl == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    // create request from template
    cJSON *req = cJSON_Duplicate(req_tpl, 1);

    // token
    std::string token = cJSON_GetObjectItem(json, "token")->valuestring;

    json_set(req, "token", token.c_str());

    char *encode = cJSON_Print(req);
    std::string content = std::string(encode);
    free(encode);
    cJSON_Delete(req);
    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_I4;
    json_req->channel = "i4";

    http_json(post_url.c_str(), content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s, json: %s", url->valuestring, content.c_str());
    return PLATFORM_OK;
}

} // namespace elf
