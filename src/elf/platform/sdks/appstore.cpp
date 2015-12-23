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

plat_base_resp* platform_appstore_on_auth(const plat_base_req *req)
{
    cJSON *code = cJSON_GetObjectItem(req->resp, "code");

    int ret = PLATFORM_OK;
    if (code == NULL || code->valueint != 0) {
        ret = PLATFORM_PARAM_ERROR;
        LOG_ERROR("platform", "appstore onAuth() falied: %d", code->valueint);
    }

    cJSON *param = cJSON_Parse(req->param.c_str());
    cJSON *userId = cJSON_GetObjectItem(param, "userId");
    cJSON_AddStringToObject(req->resp, "uid", userId->valuestring);

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->channel = req->channel;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;
    return resp;
}

int platform_appstore_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_appstore_auth: %s", param);

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

    post_url.append("&userId=");
    post_url.append(userId->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_APPSTORE;
    json_req->channel = "AppStore";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
