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

plat_base_resp* platform_vivo_on_auth(const plat_base_req *req)
{
    cJSON *msg = cJSON_GetObjectItem(req->resp, "msg");
    cJSON *stat = cJSON_GetObjectItem(req->resp, "stat");
    cJSON *userID = cJSON_GetObjectItem(req->resp, "uid");
    cJSON *email = cJSON_GetObjectItem(req->resp, "email");

    int ret = PLATFORM_OK;
    if (stat != NULL && stat->valueint != 200) {
        ret = PLATFORM_PARAM_ERROR;
        if (msg != NULL) {
            LOG_ERROR("platform", "vivo onAuth() falied: %s", msg->valuestring);
        } else {
            LOG_ERROR("platform", "vivo onAuth() falied: stat(%d)", stat->valueint);
        }
    } else {
        LOG_INFO("platform", "vivo onAuth(): userId(%s), email(%s)",
                userID->valuestring, email->valuestring);
        if (strcmp(userID->valuestring, "") == 0) {
            ret = PLATFORM_PARAM_ERROR;
        }
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

int platform_vivo_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_vivo_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_VIVO);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    //std::string encoded_token;
    //if (urlencode(token->valuestring, strlen(token->valuestring), encoded_token) < 0) {
    //    return PLATFORM_UNKOWN_ERROR;
    //}

    //char now[128] = {0};
    //sprintf(now, "%ld", elf::time_s());

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?access_token=");
    post_url.append(token->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_VIVO;
    json_req->channel = "vivo";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
