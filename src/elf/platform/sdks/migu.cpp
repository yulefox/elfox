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

plat_base_resp* platform_migu_on_auth(const plat_base_req *req)
{
    // 成功{“status”:”ok”}失败则会返回errCode（错误编号）和errMsg（错误提示）
    //
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");

    int ret = PLATFORM_OK;
    if (status == NULL || strcmp(status->valuestring, "ok") != 0) {
        ret = PLATFORM_PARAM_ERROR;
        cJSON *err = cJSON_GetObjectItem(req->resp, "errMsg");
        if (err == NULL) {
            LOG_ERROR("platform", "migu onAuth() falied: %d", status->valuestring);
        } else {
            LOG_ERROR("platform", "migu onAuth() falied: %s %s", status->valuestring, err->valuestring);
        }
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

int platform_migu_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_migu_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_MIGU);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *body = cJSON_GetObjectItem(json, "body");
    if (body == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?body=");
    post_url.append(body->valuestring);

    post_url.append("&token=");
    post_url.append(token->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_MIGU;
    json_req->channel = "migu";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
