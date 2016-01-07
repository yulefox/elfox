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

plat_base_resp* platform_msdk_on_auth(const plat_base_req *req)
{
    cJSON *ret = cJSON_GetObjectItem(req->resp, "ret");
    cJSON *msg = cJSON_GetObjectItem(req->resp, "msg");

    int code = PLATFORM_OK;
    if (ret == NULL || ret->valueint != 0 || msg == NULL) {
        code = PLATFORM_PARAM_ERROR;
        if (msg != NULL) {
            LOG_ERROR("platform", "msdk(%s) onAuth() falied: %s", req->channel.c_str(), msg->valuestring);
        } else {
            LOG_ERROR("platform", "msdk(%s) onAuth() falied", req->channel.c_str());
        }
    }

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = code;
    resp->plat_type = req->plat_type;
    resp->channel = req->channel;
    resp->resp = req->resp;

    if (code == PLATFORM_OK) {
        cJSON *param = cJSON_Parse(req->param.c_str());
        cJSON *openid = cJSON_GetObjectItem(param, "openid");
        cJSON_AddStringToObject(resp->resp, "uid", openid->valuestring);
    }

    resp->cb = req->cb;
    resp->args = req->args;

    return resp;
}

//////
// msdk

static std::string msdk_build_params(const char *appId, const char *appKey,
        const char *openid, time_t now)
{
    std::string res;
    char now_s[64];
    std::string sig;

    sprintf(now_s, "%ld", now);

    sig.append(appKey);
    sig.append(now_s);

    sig = md5((unsigned char*)sig.c_str(), sig.length());

    res.append("timestamp=");
    res.append(now_s);

    res.append("&appid=");
    res.append(appId);

    res.append("&sig=");
    res.append(sig);

    res.append("&openid=");
    res.append(openid);

    res.append("&encode=1");

    return res;
}

static void json_set(cJSON *root, const char *key, const char *val)
{
    cJSON *child = cJSON_GetObjectItem(root, key);
    if (child == NULL) {
        cJSON_AddStringToObject(root, key, val);
    } else {
        child->valuestring = strdup(val);
    }
}

int platform_qq_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_qq_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_QQ);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }


    cJSON *appId = cJSON_GetObjectItem(setting, "appId");
    if (appId == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
    if (appKey == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *req_tpl = cJSON_GetObjectItem(setting, "AuthReq");
    if (req_tpl == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    // openid
    cJSON *openId = cJSON_GetObjectItem(json, "openId");
    if (openId == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    // accessToken
    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    // userip
    cJSON *ip = cJSON_GetObjectItem(json, "ip");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    // build json for post
    // create request from template
    cJSON *req = cJSON_Duplicate(req_tpl, 1);

    // appid
    json_set(req, "appid", appId->valuestring);

    // openid
    json_set(req, "openid", openId->valuestring);

    // openkey
    json_set(req, "openkey", token->valuestring);

    // userip
    json_set(req, "userip", ip->valuestring);

    char *encode = cJSON_Print(req);
    std::string content = std::string(encode);
    free(encode);
    cJSON_Delete(req);

    time_t now = time_s();

    std::string params = msdk_build_params(appId->valuestring, appKey->valuestring,
            openId->valuestring, now);

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?");
    post_url.append(params);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_QQ;
    json_req->channel = "qq";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

int platform_weixin_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_weixin_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_WEIXIN);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }


    cJSON *appId = cJSON_GetObjectItem(setting, "appId");
    if (appId == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
    if (appKey == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *req_tpl = cJSON_GetObjectItem(setting, "AuthReq");
    if (req_tpl == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    // openid
    cJSON *openId = cJSON_GetObjectItem(json, "openId");
    if (openId == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    // accessToken
    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    // build json for post
    // create request from template
    cJSON *req = cJSON_Duplicate(req_tpl, 1);

    // openid
    json_set(req, "openid", openId->valuestring);

    // accessToken
    json_set(req, "accessToken", token->valuestring);

    char *encode = cJSON_Print(req);
    std::string content = std::string(encode);
    free(encode);
    cJSON_Delete(req);

    time_t now = time_s();

    std::string params = msdk_build_params(appId->valuestring, appKey->valuestring,
            openId->valuestring, now);

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?");
    post_url.append(params);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_WEIXIN;
    json_req->channel = "weixin";
    json_req->param = std::string(param);

    http_json(post_url.c_str(), content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
