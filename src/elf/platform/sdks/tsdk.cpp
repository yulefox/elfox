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
#include <cJSON/cJSON.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>

namespace elf {

plat_base_resp* platform_tsdk_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    //cJSON *data = cJSON_GetObjectItem(req->resp, "data");

    LOG_INFO("platform", "tsdk onAuth(): status(%s)", status->valuestring);

    int ret = PLATFORM_OK;
    if (strcmp(status->valuestring, "1") != 0) {
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

int platform_tsdk_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_tsdk_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_TSIXI);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
    if (appKey == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *userId = cJSON_GetObjectItem(json, "userId");
    if (userId == NULL || strcmp(userId->valuestring, "") == 0) {
        userId = token;
    }

    std::string sign;
    sign.append("userId=");
    sign.append(userId->valuestring);
    sign.append("token=");
    sign.append(token->valuestring);
    sign.append(appKey->valuestring);
    sign = md5((unsigned char*)sign.c_str(), sign.length());

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?userId=");
    post_url.append(userId->valuestring);

    post_url.append("&token=");
    post_url.append(token->valuestring);

    post_url.append("&sign=");
    post_url.append(sign);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_TSIXI;
    json_req->channel = "tsixi";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
