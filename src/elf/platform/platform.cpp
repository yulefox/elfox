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

    std::map<int, cJSON*>::iterator itr = s_jsons.find(type);
    if (itr != s_jsons.end() && itr->second != NULL) {
        cJSON_Delete(itr->second);
    }
    s_jsons[type] = json;
    return 0;
}

int get_channel(cJSON *setting, const char *code, std::string &channel)
{
    cJSON *channels = cJSON_GetObjectItem(setting, "channels");
    if (channels == NULL) {
        return -1;
    }
    int size = cJSON_GetArraySize(channels);
    for (int i = 0;i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(channels, i);
        if (item == NULL) {
            continue;
        }
        cJSON *ch = cJSON_GetObjectItem(item, "channel");
        cJSON *co = cJSON_GetObjectItem(item, "code");
        if (ch == NULL || co == NULL) {
            continue;
        }
        if (strcmp(co->valuestring, code) == 0) {
            channel = std::string(ch->valuestring);
            return 0;
        }
    }
    return -1;
}

cJSON* platform_get_json(int type)
{
    std::map<int, cJSON*>::iterator itr = s_jsons.find(type);
    if (itr == s_jsons.end()) {
        return NULL;
    }
    return itr->second;
}

void platform_push_resp(plat_base_resp* resp);

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    plat_base_req *base_req = static_cast<plat_base_req*>(userdata);
    if (base_req == NULL) {
        return 0;
    }

    LOG_DEBUG("net", "on cb userdata: %p", base_req->args);

    if (ptr == NULL || realsize == 0) {
        /*
        plat_base_resp *resp = E_NEW plat_base_resp;

        resp->code = PLATFORM_RESPONSE_FAILED;
        resp->plat_type = base_req->plat_type;
        resp->resp = NULL;
        resp->cb = base_req->cb;
        resp->args = base_req->args;

        E_DELETE base_req;

        // push resp
        s_resps.push(resp);
        */
        return realsize;
    }

    plat_base_resp *base_resp = NULL;

    if (base_req->plat_type == PLAT_LJ) {
        cJSON *resp = cJSON_Parse(base_req->param.c_str());
        std::string st((char*)ptr, realsize);
        cJSON_AddStringToObject(resp, "status", st.c_str());

        base_req->resp = resp;
        base_resp = platform_lj_on_auth(base_req);
        E_DELETE base_req;
        if (strcmp((char*)ptr, "true") != 0 || strcmp((char*)ptr, "false") != 0) {
            realsize = 0;
        }
    } else if (base_req->plat_type == PLAT_1SDK) {
        cJSON *resp = cJSON_Parse(base_req->param.c_str());
        std::string st((char*)ptr, realsize);
        cJSON_AddStringToObject(resp, "status", st.c_str());

        base_req->resp = resp;
        base_resp = platform_1sdk_on_auth(base_req);
        E_DELETE base_req;
    } else {
        bool unquote = false;
        if (base_req->plat_type == PLAT_ANZHI) {
            unquote = true;
        }
        if (base_req->push_resp(ptr, realsize, unquote)) {
            switch (base_req->plat_type) {
            case PLAT_PP:
                base_resp = platform_pp_on_auth(base_req);
                break;
            case PLAT_UC:
                base_resp = platform_uc_on_auth(base_req);
                break;
            case PLAT_I4:
                base_resp = platform_i4_on_auth(base_req);
                break;
            case PLAT_HUAWEI:
                base_resp = platform_huawei_on_auth(base_req);
                break;
            case PLAT_VIVO:
                base_resp = platform_vivo_on_auth(base_req);
                break;
            case PLAT_ANZHI:
                base_resp = platform_anzhi_on_auth(base_req);
                break;
            case PLAT_QQ:
            case PLAT_WEIXIN:
                base_resp = platform_msdk_on_auth(base_req);
                break;
            case PLAT_APPSTORE:
                base_resp = platform_appstore_on_auth(base_req);
                break;
            case PLAT_MIGU:
                base_resp = platform_migu_on_auth(base_req);
                break;
            }
            E_DELETE base_req;
        }
    }
    if (base_resp != NULL) {
        platform_push_resp(base_resp);
    }
    return realsize;
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
    case PLAT_HUAWEI:
        return platform_huawei_auth(data, cb, args);
    case PLAT_VIVO:
        return platform_vivo_auth(data, cb, args);
    case PLAT_ANZHI:
        return platform_anzhi_auth(data, cb, args);
    case PLAT_QQ:
        return platform_qq_auth(data, cb, args);
    case PLAT_WEIXIN:
        return platform_weixin_auth(data, cb, args);
    case PLAT_APPSTORE:
        return platform_appstore_auth(data, cb, args);
    case PLAT_MIGU:
        return platform_migu_auth(data, cb, args);
    default:
        return PLATFORM_TYPE_ERROR;
        break;
    }
    return PLATFORM_OK;
}

void platform_push_resp(plat_base_resp* resp)
{
    s_resps.push(resp);
}

int platform_proc() {
    std::deque<plat_base_resp*>::iterator itr;
    std::deque<plat_base_resp*> resps;

    s_resps.swap(resps);
    for (itr = resps.begin();itr != resps.end(); ++itr) {
        plat_base_resp *resp = *itr;
        if (resp->cb != NULL) {
            resp->cb(resp->plat_type, resp->channel, resp->code, resp->resp, resp->args);
        }

        if (resp->resp != NULL) {
            cJSON_Delete(resp->resp);
        }
        E_DELETE resp;
    }
    return 0;
}

} // namespace elf
