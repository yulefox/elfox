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

/*
    // resp
     {
        "data": {
            "app_id": 138483919580948, 
            "application": "Social Cafe", 
            "expires_at": 1352419328, 
            "is_valid": true, 
            "issued_at": 1347235328, 
            "metadata": {
                "sso": "iphone-safari"
            }, 
            "scopes": [
                "email", 
                "publish_actions"
            ], 
            "user_id": 1207059
        }
    }
*/

plat_base_resp* platform_facebook_on_auth(const plat_base_req *req)
{
    int ret = PLATFORM_OK;
    cJSON *data = cJSON_GetObjectItem(req->resp, "data");

    if (data == NULL) {
        ret = PLATFORM_PARAM_ERROR;
    } else {
        cJSON *is_valid = cJSON_GetObjectItem(data, "is_valid");
        if (is_valid == NULL || is_valid->valueint != 1) {
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

/*
 *    "ios" : {
        "appID" : "1157500974315562",
        "appKey" : "d6b6b720ccd00e315e9fbd22b4075378"
    },
*/

int platform_facebook_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_facebook_auth: %s", param);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_FACEBOOK);
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

    bool ios = false;
    cJSON *plat = cJSON_GetObjectItem(json, "platform");
    if (plat && !strcmp(plat->valuestring, "ios")) {
        ios = true;
    }

    cJSON *plat_info = NULL;

    if (ios) {
        plat_info = cJSON_GetObjectItem(setting, "ios");
    } else {
        plat_info = cJSON_GetObjectItem(setting, "android");
    }

    if (plat_info == NULL) {
        LOG_DEBUG("net", "%s", "no found platform info...");
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
    if (appKey == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    /*
     *GET graph.facebook.com/debug_token?
     input_token={token-to-inspect}
     &access_token={app-token-or-admin-token}
     */

    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?input_token=");
    post_url.append(token->valuestring);

    post_url.append("&access_token=");
    post_url.append(appKey->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_FACEBOOK;
    json_req->channel = "facebook";
    json_req->param = std::string(param);

    http_json(HTTP_GET, post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

} // namespace elf
