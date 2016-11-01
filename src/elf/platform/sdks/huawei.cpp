#include <elf/elf.h>
#include <elf/config.h>
#include <elf/net/http.h>
#include <elf/md5.h>
#include <elf/base64.h>
#include <elf/rsa.h>
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

const static char *HUAWEI_PUB_KEY = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmKLBMs2vXosqSR2rojMz\nioTRVt8oc1ox2uKjyZt6bHUK0u+OpantyFYwF3w1d0U3mCF6rGUnEADzXiX/2/Rg\nLQDEXRD22er31ep3yevtL/r0qcO8GMDzy3RJexdLB6z20voNM551yhKhB18qyFes\niPhcPKBQM5dnAOdZLSaLYHzQkQKANy9fYFJlLDo11I3AxefCBuoG+g7ilti5qgpb\nkm6rK2lLGWOeJMrF+Hu+cxd9H2y3cXWXxkwWM1OZZTgTq3Frlsv1fgkrByJotDpR\ne8SwkiVuRycR0AHsFfIsuZCFwZML16EGnHqm2jLJXMKIBgkZTzL8Z+201RmOheV4\nAQIDAQAB\n-----END PUBLIC KEY-----\n";

plat_base_resp* platform_huawei_on_auth(const plat_base_req *req)
{
    cJSON *error = cJSON_GetObjectItem(req->resp, "error");
    cJSON *userID = cJSON_GetObjectItem(req->resp, "userID");
    //cJSON *username = cJSON_GetObjectItem(req->resp, "userName");
    //cJSON *userState = cJSON_GetObjectItem(req->resp, "userState");
    //cJSON *userValidStatus = cJSON_GetObjectItem(req->resp, "userValidStatus");

    int ret = PLATFORM_OK;
    if (userID == NULL) {
        ret = PLATFORM_PARAM_ERROR;
        if (error != NULL) {
            LOG_ERROR("platform", "huawei onAuth() falied: %s", error->valuestring);
        }
    } else {
        //LOG_INFO("platform", "huawei onAuth(): userId(%s), username(%s), userState(%d), userValidStatus(%d)",
        //       userID->valuestring, username->valuestring,
        //       userState->valueint, userValidStatus->valueint);

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

int platform_huawei_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_huawei_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_HUAWEI);
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

    std::string encoded_token;
    if (urlencode(token->valuestring, strlen(token->valuestring), encoded_token) < 0) {
        return PLATFORM_UNKOWN_ERROR;
    }

    char now[128] = {0};
    sprintf(now, "%ld", elf::time_s());

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?nsp_svc=OpenUP.User.getInfo");
    post_url.append("&nsp_ts=");
    post_url.append(now);
    post_url.append("&access_token=");
    post_url.append(encoded_token);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_HUAWEI;
    json_req->channel = "huawei";
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;

}

int platform_huawei_auth_v2(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_huawei_auth_v2: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_HUAWEI);
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

    std::string info = base64_decode(token->valuestring, strlen(token->valuestring), false);
    cJSON *token_info = cJSON_Parse(info.c_str());
    if (token_info == NULL) {
        LOG_ERROR("net", "%s", "huawei sdk v2: tokeninfo failed.");
        return PLATFORM_UNKOWN_ERROR;
    }

    cJSON *playerId = cJSON_GetObjectItem(token_info, "playerId");
    cJSON *appId = cJSON_GetObjectItem(token_info, "appId");
    cJSON *ts = cJSON_GetObjectItem(token_info, "ts");
    cJSON *sign = cJSON_GetObjectItem(token_info, "gameAuthSign");

    int ret = PLATFORM_PARAM_ERROR;
    std::string ctx = std::string(appId->valuestring) + std::string(ts->valuestring) + std::string(playerId->valuestring); 

    if (1== RSAVerify(ctx, sign->valuestring, HUAWEI_PUB_KEY)) {
        LOG_INFO("net", "%s", "auth ok...");
        if (cb && cb(PLAT_HUAWEI, "huawei", PLATFORM_OK, token_info, args) == 0) {
            ret = PLATFORM_OK;
        }
    }

    cJSON_Delete(token_info);
    cJSON_Delete(json);

    return ret;
}

} // namespace elf
