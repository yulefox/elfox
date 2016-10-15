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
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");

    int ret = PLATFORM_OK;
    if (status == NULL || strcmp(status->valuestring, "false") == 0) {
        ret = PLATFORM_PARAM_ERROR;
        if (status != NULL) {
            LOG_ERROR("platform", "appstore onAuth() falied: %s", status->valuestring);
        }
    }

    cJSON *userId = cJSON_GetObjectItem(req->resp, "userId");
    if (userId == NULL || strcmp(userId->valuestring, "") == 0) {
        ret = PLATFORM_PARAM_ERROR;
    } else {
        json_set(req->resp, "uid", userId->valuestring);
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

int platform_appstore_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_appstore_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    int plat = PLAT_APPSTORE;
    cJSON *ch_node = cJSON_GetObjectItem(json, "channel");
    std::string channel;
    if (ch_node != NULL) {
        channel = ch_node->valuestring;
        if (strcmp(ch_node->valuestring, "AppStore") == 0) {
            plat = PLAT_APPSTORE;
        } else if (strcmp(ch_node->valuestring, "iosintsg") == 0) {
            plat = PLAT_IOSINTSG;
        } else if (strcmp(ch_node->valuestring, "iosintmy") == 0) {
            plat = PLAT_IOSINTMY;
        } else if (strcmp(ch_node->valuestring, "pjmy") == 0) {
            plat = PLAT_PJMY;
        } else if (strcmp(ch_node->valuestring, "pjsg") == 0) {
            plat = PLAT_PJSG;
        } else {
            LOG_ERROR("net", "invalid channel: %s", ch_node->valuestring);
        }
    }

    cJSON *setting = platform_get_json(plat);
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


    //cJSON *userId = cJSON_GetObjectItem(json, "userId");
    //if (userId == NULL) {
    //    return PLATFORM_PARAM_ERROR;
    //}

    cJSON *token = cJSON_GetObjectItem(json, "token");
    if (token == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    //http://42.62.77.103/gamechecktoken?game=jfjh&token=xxxx
    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?game=jfjh");

    post_url.append("&token=");
    post_url.append(token->valuestring);

    //post_url.append("&userId=");
    //post_url.append(userId->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = plat;
    json_req->channel = channel;
    json_req->param = std::string(param);

    http_json(HTTP_POST, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
