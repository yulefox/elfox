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

plat_base_resp* platform_damai_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *data = cJSON_GetObjectItem(req->resp, "data");

    LOG_INFO("platform", "1sdk onAuth(): status(%d), data(%s)",
            status->valueint, data->valuestring);

    int ret = PLATFORM_OK;
    if (status->valueint != 1) {
        ret = PLATFORM_PARAM_ERROR;
    }

    cJSON *param = cJSON_Parse(req->param.c_str());
    cJSON *userId = cJSON_GetObjectItem(param, "userId");

    json_set(req->resp, "uid", userId->valuestring);

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->channel = req->channel;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    return resp;
}

int platform_damai_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_damai_auth: %s", param);

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

    cJSON *app = cJSON_GetObjectItem(setting, "appId");
    if (app == NULL) {
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

    const char *fmt = "{\"id\": %d,\"appid\": %d,\"username\":\"%s\",\"token\":\"%s\"}"; 
    char json_data[2048];
    sprintf(json_data, fmt, time_s(), app->valueint, userId->valuestring, token->valuestring);

    std::string post_url;
    post_url.append(url->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_DAMAI;
    json_req->channel = "damai";
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), json_data, write_callback, json_req);

    LOG_DEBUG("net", "url: %s %s", post_url.c_str(), json_data);
    return PLATFORM_OK;
}

} // namespace elf
