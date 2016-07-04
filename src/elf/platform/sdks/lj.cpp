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

plat_base_resp* platform_lj_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *userid = cJSON_GetObjectItem(req->resp, "userId");

    LOG_INFO("platform", "lj onAuth(): status(%s), userid(%s)",
            status->valuestring, userid->valuestring);

    int ret = PLATFORM_OK;
    if (strcmp(status->valuestring, "true") != 0) {
        ret = PLATFORM_PARAM_ERROR;
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

int platform_lj_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_lj_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_LJ);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *productCode = cJSON_GetObjectItem(setting, "productCode");
    if (productCode == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *userId = cJSON_GetObjectItem(json, "userId");
    if (userId == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *channelCode = cJSON_GetObjectItem(json, "channelCode");
    if (channelCode == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    std::string channel;
    if (get_channel(setting, channelCode->valuestring, channel) < 0) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?productCode=");
    post_url.append(productCode->valuestring);

    post_url.append("&token=");
    post_url.append(token->valuestring);

    post_url.append("&channel=");
    post_url.append(channelCode->valuestring);

    post_url.append("&userId=");
    post_url.append(userId->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_LJ;
    json_req->channel = channel;
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
