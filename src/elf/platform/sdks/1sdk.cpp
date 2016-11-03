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

plat_base_resp* platform_1sdk_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *userid = cJSON_GetObjectItem(req->resp, "userId");

    LOG_INFO("platform", "1sdk onAuth(): status(%s), userid(%s)",
            status->valuestring, userid->valuestring);

    int ret = PLATFORM_OK;
    if (strcmp(status->valuestring, "0") != 0) {
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

int platform_1sdk_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_1sdk_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_1SDK);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *app = cJSON_GetObjectItem(setting, "app");
    if (app == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *sdk = cJSON_GetObjectItem(json, "channelCode");
    if (sdk == NULL) {
        sdk = cJSON_GetObjectItem(json, "channelId");
        if (sdk == NULL) {
            return PLATFORM_PARAM_ERROR;
        }
    }

    std::string channel;
    if (get_channel(setting, sdk->valuestring, channel) < 0) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *ch = cJSON_GetObjectItem(json, "channel");
    if (ch == NULL) {
        LOG_ERROR("net", "%s", "no found channel in the json content");
        return PLATFORM_PARAM_ERROR;
    }
    if (0 == strcmp(ch ->valuestring, "1sdk2")) {
        app = cJSON_GetObjectItem(setting, "app2");
        if (app == NULL) {
            LOG_ERROR("net", "%s", "no found appId2");
            return PLATFORM_PARAM_ERROR;
        }
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *userId = cJSON_GetObjectItem(json, "userId");
    if (userId == NULL || strcmp(userId->valuestring, "") == 0) {
        userId = token;
    }

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?sdk=");
    post_url.append(sdk->valuestring);

    post_url.append("&app=");
    post_url.append(app->valuestring);

    post_url.append("&uin=");
    post_url.append(userId->valuestring);

    post_url.append("&sess=");
    post_url.append(token->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_1SDK;
    json_req->channel = channel;
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
