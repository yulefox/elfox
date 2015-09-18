#include <elf/elf.h>
#include <elf/config.h>
#include <elf/net/http.h>
#include <elf/md5.h>
#include <elf/json.h>
#include <elf/time.h>
#include <elf/log.h>
#include <elf/pc.h>
#include <elf/platform/platform.h>
#include <cJSON/cJSON.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>

namespace elf {

enum plat_req_type {
    PLAT_REQ_JSON,
};

struct plat_base_req {
    int plat_type;
    int type;
    std::string content;
    std::string param;
    void *args;
    cJSON *resp;
    auth_cb cb;

    plat_base_req() : type(0), args(NULL), cb(NULL) {}
    virtual ~plat_base_req() {}

    plat_base_req(int _type, auth_cb _cb, void *_args)
    : type(_type)
    , args(_args)
    , cb(_cb) {}
    virtual bool push_resp(void *ptr, size_t size) = 0;
};

struct plat_base_resp {
    int plat_type;
    int type;
    int code;
    auth_cb cb;
    cJSON *resp;
    void *args;

    plat_base_resp()
    : plat_type(0)
    , type(0)
    , code(0)
    , cb(NULL)
    , resp(NULL)
    , args(NULL) {}
};

struct plat_json_req : public plat_base_req {
    plat_json_req() : plat_base_req(PLAT_REQ_JSON, NULL, NULL) {}
    plat_json_req(auth_cb cb, void *args) : plat_base_req(PLAT_REQ_JSON, cb, args) {}
    virtual ~plat_json_req() {}

    bool push_resp(void *ptr, size_t size) {
        content.append((char*)ptr, size);
        resp = cJSON_Parse(content.c_str());
        if (resp == NULL) {
            return false;
        }
        return true;
    }
};

static std::map<int, cJSON*> s_jsons;
static xqueue<plat_base_resp*> s_resps;
static void platform_pp_on_auth(const plat_base_req *req);
static void platform_uc_on_auth(const plat_base_req *req);
static void platform_i4_on_auth(const plat_base_req *req);
static void platform_lj_on_auth(const plat_base_req *req);
static void platform_1sdk_on_auth(const plat_base_req *req);

int platform_init()
{
    s_jsons.clear();
    return 0;
}

int platform_fini()
{
    std::map<int, cJSON*>::iterator itr;
    for (itr = s_jsons.begin();itr != s_jsons.end(); ++itr) {
        cJSON_Delete(itr->second);
    }
    return 0;
}

int platform_load(int type, const char *proto)
{
    assert(proto);

    std::fstream fs(proto, std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("json",
                "Can NOT open file %s.", proto);
        return -1;
    }

    std::stringstream iss;

    iss << fs.rdbuf();
    cJSON *json = cJSON_Parse(iss.str().c_str());
    if (json == NULL) {
        LOG_ERROR("json",
                "Can NOT parse json file %s.", proto);
        return -1;
    }
    s_jsons.insert(std::make_pair<int, cJSON*>(type, json));
    return 0;
}

static cJSON* platform_get_json(int type)
{
    std::map<int, cJSON*>::iterator itr = s_jsons.find(type);
    if (itr == s_jsons.end()) {
        return NULL;
    }
    return itr->second;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    plat_base_req *base_req = static_cast<plat_base_req*>(userdata);
    if (base_req == NULL) {
        return 0;
    }

    LOG_DEBUG("net", "on cb userdata: %p", base_req->args);

    if (ptr == NULL || realsize == 0) {
        plat_base_resp *resp = E_NEW plat_base_resp;

        resp->code = PLATFORM_RESPONSE_FAILED;
        resp->plat_type = base_req->plat_type;
        resp->resp = NULL;
        resp->cb = base_req->cb;
        resp->args = base_req->args;

        E_DELETE base_req;

        // push resp
        s_resps.push(resp);
        return realsize;
    }

    if (base_req->plat_type == PLAT_LJ) {
        cJSON *resp = cJSON_Parse(base_req->param.c_str());
        std::string st((char*)ptr, realsize);
        cJSON_AddStringToObject(resp, "status", st.c_str());

        base_req->resp = resp;
        platform_lj_on_auth(base_req);
        E_DELETE base_req;
    } else if (base_req->plat_type == PLAT_1SDK) {
        cJSON *resp = cJSON_Parse(base_req->param.c_str());
        std::string st((char*)ptr, realsize);
        cJSON_AddStringToObject(resp, "status", st.c_str());

        base_req->resp = resp;
        platform_1sdk_on_auth(base_req);
        E_DELETE base_req;
    } else {
        if (base_req->push_resp(ptr, realsize)) {
            switch (base_req->plat_type) {
            case PLAT_PP:
                platform_pp_on_auth(base_req);
                break;
            case PLAT_UC:
                platform_uc_on_auth(base_req);
                break;
            case PLAT_I4:
                platform_i4_on_auth(base_req);
                break;
            }
            E_DELETE base_req;
        }
    }
    return realsize;
}

static void platform_pp_on_auth(const plat_base_req *req)
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
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    // push resp
    s_resps.push(resp);
}

static void platform_uc_on_auth(const plat_base_req *req)
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
    case 99: // timeout
        ret = PLATFORM_RESPONSE_FAILED;
        break;
    default:
        ret = PLATFORM_UNKOWN_ERROR;
        break;
    }

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    // push resp
    s_resps.push(resp);
}

static void  platform_i4_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *username = cJSON_GetObjectItem(req->resp, "username");
    cJSON *userid = cJSON_GetObjectItem(req->resp, "userid");

    LOG_INFO("platform", "i4 onAuth(): status(%d), username(%s), userid(%d)",
            status->valueint, username->valuestring, userid->valueint);

    int ret = PLATFORM_OK;
    switch (status->valueint) {
    case 0: // success
        ret = PLATFORM_OK;
        break;
    case 1: // token invalid
        ret = PLATFORM_PARAM_ERROR;
        break;
    case 2: // user not exist
        ret = PLATFORM_USER_NOT_EXIEST;
        break;
    case 3: // timeout
        ret = PLATFORM_RESPONSE_FAILED;
        break;
    default:
        ret = PLATFORM_UNKOWN_ERROR;
        break;
    }

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    // push resp
    s_resps.push(resp);

}

static void  platform_lj_on_auth(const plat_base_req *req)
{
    cJSON *status = cJSON_GetObjectItem(req->resp, "status");
    cJSON *userid = cJSON_GetObjectItem(req->resp, "userId");

    LOG_INFO("platform", "lj onAuth(): status(%s), userid(%s)",
            status->valuestring, userid->valuestring);

    int ret = PLATFORM_OK;
    if (strcmp(status->valuestring, "true") != 0) {
        ret = PLATFORM_PARAM_ERROR;
    }

    plat_base_resp *resp = E_NEW plat_base_resp;
    resp->code = ret;
    resp->plat_type = req->plat_type;
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    // push resp
    s_resps.push(resp);
}

static void platform_1sdk_on_auth(const plat_base_req *req)
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
    resp->resp = req->resp;
    resp->cb = req->cb;
    resp->args = req->args;

    // push resp
    s_resps.push(resp);
}

static int platform_pp_auth(const char *param, auth_cb cb, void *args)
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

    http_json(url->valuestring, content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s, json: %s", url->valuestring, content.c_str());
    return PLATFORM_OK;
}

static int platform_i4_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_i4_auth: %p", args);

    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_I4);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    std::string post_url;
    post_url.append(url->valuestring);
    post_url.append("?token=");
    post_url.append(cJSON_GetObjectItem(json, "token")->valuestring);

    cJSON *req_tpl = cJSON_GetObjectItem(setting, "AuthReq");
    if (req_tpl == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    // create request from template
    cJSON *req = cJSON_Duplicate(req_tpl, 1);

    // token
    std::string token = cJSON_GetObjectItem(json, "token")->valuestring;
    cJSON *req_token = cJSON_GetObjectItem(req, "token");
    if (req_token == NULL) {
        cJSON_AddStringToObject(req, "token", token.c_str());
    }
    req_token->valuestring = strdup(token.c_str());

    char *encode = cJSON_Print(req);
    std::string content = std::string(encode);
    free(encode);
    cJSON_Delete(req);
    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_I4;

    http_json(post_url.c_str(), content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s, json: %s", url->valuestring, content.c_str());
    return PLATFORM_OK;
}

static int platform_lj_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_lj_auth: %s", param);

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

    cJSON *channelCode = cJSON_GetObjectItem(json, "channelCode");
    if (channelCode == NULL) {
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

    post_url.append("&channel=");
    post_url.append(channelCode->valuestring);

    post_url.append("&userId=");
    post_url.append(userId->valuestring);

    cJSON_Delete(json);

    // do post request
    plat_json_req *json_req = E_NEW plat_json_req(cb, args);
    json_req->plat_type = PLAT_LJ;
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

static int platform_1sdk_auth(const char *param, auth_cb cb, void *args)
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
    json_req->param = std::string(param);

    http_json(post_url.c_str(), "", write_callback, json_req);

    LOG_DEBUG("net", "url: %s", post_url.c_str());
    return PLATFORM_OK;
}

static int platform_uc_auth(const char *param, auth_cb cb, void *args)
{
    LOG_DEBUG("net", "platform_uc_auth: %p", args);


    cJSON *json = cJSON_Parse(param);
    if (json == NULL) {
        return PLATFORM_PARAM_ERROR;
    }

    cJSON *setting = platform_get_json(PLAT_UC);
    if (setting == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    
    cJSON *url = cJSON_GetObjectItem(setting, "URL");
    if (url == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appId = cJSON_GetObjectItem(setting, "gameId");
    if (appId == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    cJSON *appKey = cJSON_GetObjectItem(setting, "apiKey");
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
    signtx.append("sid=");
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
    json_req->plat_type = PLAT_UC;

    http_json(url->valuestring, content.c_str(), write_callback, json_req);

    LOG_DEBUG("net", "url: %s, json: %s", url->valuestring, content.c_str());
    return PLATFORM_OK;
}


int platform_auth(int plat_type, const char *data,
        auth_cb cb, void *args) {
    switch (plat_type) {
    case PLAT_PP:
        return platform_pp_auth(data, cb, args);
    case PLAT_I4:
        return platform_i4_auth(data, cb, args);
    case PLAT_LJ:
        return platform_lj_auth(data, cb, args);
    case PLAT_1SDK:
        return platform_1sdk_auth(data, cb, args);
    case PLAT_UC:
        return platform_uc_auth(data, cb, args);
    default:
        return PLATFORM_TYPE_ERROR;
        break;
    }
    return PLATFORM_OK;
}

int platform_proc() {
    std::deque<plat_base_resp*>::iterator itr;
    std::deque<plat_base_resp*> resps;

    s_resps.swap(resps);
    for (itr = resps.begin();itr != resps.end(); ++itr) {
        plat_base_resp *resp = *itr;
        if (resp->cb != NULL) {
            resp->cb(resp->plat_type, resp->code, resp->resp, resp->args);
        }

        if (resp->resp != NULL) {
            cJSON_Delete(resp->resp);
        }
        E_DELETE resp;
    }
    return 0;
}

} // namespace elf
