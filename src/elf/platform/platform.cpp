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
    auth_cb cb;
    cJSON *resp;
    void *args;
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
    plat_base_req *base_req = static_cast<plat_base_req*>(userdata);
    if (base_req == NULL) {
        return 0;
    }
    size_t realsize = size + nmemb;
    if (base_req->push_resp(ptr, realsize)) {
        plat_base_resp *resp = E_NEW plat_base_resp;

        resp->plat_type = base_req->plat_type;
        resp->resp = base_req->resp;
        resp->cb = base_req->cb;
        resp->args = base_req->args;

        E_DELETE base_req;

        // push resp
        s_resps.push(resp);
        return 0;
    }
    return realsize;
}

static int plat_form_pp_auth(const char *param, auth_cb cb, void *args)
{
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
    sid->valuestring = strdup(cJSON_GetObjectItem(json, "sid")->valuestring);

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

    LOG_DEBUG("net", "url: %s, json: %s\n", url->valuestring, content.c_str());
    return PLATFORM_OK;
}

int platform_auth(int plat_type, const char *data,
        auth_cb cb, void *args) {
    switch (plat_type) {
    case PLAT_PP:
        return plat_form_pp_auth(data, cb, args);
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
            resp->cb(resp->plat_type, resp->resp, resp->args);
        }
    }
    return 0;
}

} // namespace elf
