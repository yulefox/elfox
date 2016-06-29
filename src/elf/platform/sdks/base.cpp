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

namespace elf {
    bool plat_json_req::push_resp(void *ptr, size_t size, bool unquote) {
        content.append((char*)ptr, size);
        if (unquote) {
            std::string buf = content;
            for (size_t i = 0;i < buf.size(); i++) {
                if (buf[i] == '\'') {
                    buf[i] = '\"';
                }
            }
            resp = cJSON_Parse(buf.c_str());
            LOG_ERROR("net", "buf: %s", buf.c_str());
        } else {
            resp = cJSON_Parse(content.c_str());
        }
        if (resp == NULL) {
            return false;
        }
        return true;
    }

    //
    void json_set(cJSON *root, const char *key, const char *val)
    {
        cJSON *child = cJSON_GetObjectItem(root, key);
        if (child == NULL) {
            cJSON_AddStringToObject(root, key, val);
        } else {
            if (child->valuestring) free(child->valuestring);
            child->valuestring = strdup(val);
        }
    }
}

