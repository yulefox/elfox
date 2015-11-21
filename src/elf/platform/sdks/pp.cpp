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

plat_base_resp* platform_pp_on_auth(const plat_base_req *req)
{
    cJSON *state = cJSON_GetObjectItem(req->resp, "state");
    cJSON *code = cJSON_GetObjectItem(state, "code");
    cJSON *msg = cJSON_GetObjectItem(state, "msg");

    LOG_INFO("platform", "pp onAuth(): code(%d), msg(%s)",
            code->valueint, msg->valuestring);

    int ret = PLATFORM_OK;
    switch (code->valueint) {
    case 1: // success
        ret = PLATFORM_OK;
        break;
    case 10: // param invalid
        ret = PLATFORM_PARAM_ERROR;
        break;
    case 11: // not loginin
        ret = PLATFORM_USER_NOT_LOGININ;
        break;
    case 9: // timeout
        ret = PLATFORM_RESPONSE_FAILED;
        break;
    default:
        ret = PLATFORM_UNKOWN_ERROR;
        break;
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

int platform_pp_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_pp_auth: %p", args);


    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_PP);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appId = cJSON_GetObjectItem(setting, "AppId");
    if (appId == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "AppKey");
    if (appKey == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *req_tpl = cJSON_GetObjectItem(setting, "AuthReq");
    if (req_tpl == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    // create request from template
    cJSON *req = cJSON_Duplicate(req_tpl, 1);

    // id
    long now = time_s();
    cJSON *id = cJSON_GetObjectItem(req, "id");
    if (id == NULL) {
        cJSON_AddNumberToObject(req, "id", now);
    } else {
        id->valueint = now;
        cJSON_SetIntValue(id, now);
    }

    // data/sid
    cJSON *data = cJSON_GetObjectItem(req, "data");
    cJSON *sid = cJSON_GetObjectItem(data, "sid");
    sid->valuestring = strdup(cJSON_GetObjectItem(json, "token")->valuestring);

    // game/appId
    cJSON *game = cJSON_GetObjectItem(req, "game");
    cJSON *gameId = cJSON_GetObjectItem(game, "gameId");
    cJSON_SetIntValue(gameId, appId->valueint);

    // sign data
    std::string signtx;
    signtx.append(sid->valuestring);
    signtx.append(appKey->valuestring);
    std::string md5sum = md5((unsigned char*)signtx.c_str(), signtx.length());
    cJSON *sign = cJSON_GetObjectItem(req, "sign");
    sign->valuestring = strdup(md5sum.c_str());

    //char *encode = cJSON_PrintUnformatted(req);
    char *encode = cJSON_Print(req);
    std::string content = std::string(encode);
    free(encode);
    cJSON_Delete(req);
    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_PP;
    json_req->channel = "pp";

    http_json(url->valuestring, content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s, json: %s", url->valuestring, content.c_str());
    return PLATFORM_OK;
}

} // namespace elf
