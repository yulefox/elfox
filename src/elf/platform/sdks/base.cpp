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
        LOG_ERROR("net", "%s", "push_resp");
        content.append((char*)ptr, size);
        LOG_ERROR("net", "unquote: %d", unquote ? 1 : 0);
        if (unquote) {
            LOG_ERROR("net", "%s", "unquote");

            std::string buf = content;
            LOG_ERROR("net", "buf: %s", buf.c_str());
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
}

