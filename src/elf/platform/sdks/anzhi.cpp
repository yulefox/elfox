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

plat_base_resp* platform_anzhi_on_auth(const plat_base_req *req)
{
    cJSON *sc = cJSON_GetObjectItem(req->resp, "sc");
    cJSON *st = cJSON_GetObjectItem(req->resp, "st");
    cJSON *time = cJSON_GetObjectItem(req->resp, "time");
    cJSON *msg = cJSON_GetObjectItem(req->resp, "msg");

    int ret = PLATFORM_OK;
    if (sc == NULL || strcmp(sc->valuestring, "1") != 0 || msg == NULL) {
        ret = PLATFORM_PARAM_ERROR;
        if (st != NULL) {
            LOG_ERROR("platform", "anzhi onAuth() falied: %s", st->valuestring);
        } else {
            LOG_ERROR("platform", "%s", "anzhi onAuth() falied");
        }
    } else {
        std::string ctx = base64_decode(msg->valuestring, strlen(msg->valuestring), false);
        for (size_t i = 0;i < ctx.size(); i++) {
            if (ctx[i] == '\'') {
                ctx[i] = '\"';
            }
        }
        cJSON *json = cJSON_Parse(ctx.c_str());
        if (json == NULL) {
            LOG_ERROR("platform", "%s", "anzhi onAuth() falied");
            ret = PLATFORM_PARAM_ERROR;
        } else {
            cJSON *uid = cJSON_GetObjectItem(json, "uid");
            if (uid == NULL || strcmp(uid->valuestring, "") == 0) {
                ret = PLATFORM_PARAM_ERROR;
            } else {
                LOG_INFO("platform", "anzhi onAuth(): userId(%s) time(%s)",
                        uid->valuestring, time->valuestring);
                json_set(req->resp, "uid", uid->valuestring);
            }
        }
        cJSON_Delete(json);
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

int platform_anzhi_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_anzhi_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_ANZHI);
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

    cJSON *appSecret = cJSON_GetObjectItem(setting, "appSecret");
    if (appSecret == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *userId = cJSON_GetObjectItem(json, "userId");
    if (userId == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    //appkey+account+sid+appsecret
    std::string ctx = appKey->valuestring;
    ctx.append(userId->valuestring);
    ctx.append(token->valuestring);
    ctx.append(appSecret->valuestring);

    std::string sign = base64_encode(ctx.c_str(), ctx.size(), false);

    struct tm ctm;
    time64_t now = time_ms();
    char sec[64], now_s[64];
    time_t tm = (time_t)(now / 1000);

    localtime_r(&tm, &ctm);
    strftime(sec, sizeof(sec), "%Y%m%d%H%M%S", &ctm);
    sprintf(now_s, "%s%lld", sec, now - tm * 1000);


    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?time=");
    post_url.append(now_s);

    post_url.append("&appkey=");
    post_url.append(appKey->valuestring);

    post_url.append("&account=");
    post_url.append(userId->valuestring);

    post_url.append("&sid=");
    post_url.append(token->valuestring);

    post_url.append("&sign=");
    post_url.append(sign);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_ANZHI;
    json_req->channel = "anzhi";
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
