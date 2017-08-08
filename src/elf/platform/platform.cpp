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
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <jansson.h>
#include <jwt.h>
#include <errno.h>

namespace elf {

static json_t *s_platform_setting;

int platform_init(const char *configfile)
{

    std::fstream fs(configfile, std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("json", "Can NOT open file %s.", configfile);
        return -1;
    }

    std::stringstream iss;

    iss << fs.rdbuf();

    s_platform_setting = json_loads(iss.str().c_str(), 0, NULL);
    if (s_platform_setting == NULL) {
        LOG_ERROR("json", "Can NOT parse json file %s.", configfile);
        return -1;
    }

    return 0;
}

int platform_fini()
{
    if (s_platform_setting != NULL) {
        json_decref(s_platform_setting);
    }
    return 0;
}

int platform_auth(const char *token, platform_user_t &puser)
{
    jwt_t *jwt = NULL;
    int ret;

    json_t *key = json_object_get(s_platform_setting, "public_key");
    if (key == NULL) {
        return PLATFORM_SETTING_ERROR;
    }
    json_t *app_id_node = json_object_get(s_platform_setting, "app_id");
    if (app_id_node == NULL) {
        return PLATFORM_SETTING_ERROR;
    }

    const char *key_str = json_string_value(key);
    size_t key_len = json_string_length(key);
    if (key_str == NULL || key_len <= 0) {
        return PLATFORM_SETTING_ERROR;
    }

    ret = jwt_decode(&jwt, token, (const unsigned char*)key_str, (int)key_len);
    if (ret != 0) {
        LOG_ERROR("json", "jwt decode failed: %d %d", ret, errno);
        return PLATFORM_TOKEN_INVALID;
    }

    if (json_integer_value(app_id_node) != jwt_get_grant_int(jwt, "app_id")) {
        jwt_free(jwt);
        return PLATFORM_TOKEN_INVALID;
    }

    if (time_s() > jwt_get_grant_int(jwt, "exp")) {
        jwt_free(jwt);
        return PLATFORM_TOKEN_EXPIRED;
    }

    char *uid_str = jwt_get_grants_json(jwt, "uid");
    const char *platform = jwt_get_grant(jwt, "platform");
    const char *channel = jwt_get_grant(jwt, "channel");
    const char *sdk = jwt_get_grant(jwt, "sdk");
    if (uid_str == NULL || platform == NULL || channel == NULL || sdk == NULL) {
        jwt_free(jwt);
        return PLATFORM_TOKEN_INVALID;
    }

    int64_t uid = strtoll(uid_str, NULL, 10);
    if (errno == EINVAL || errno == ERANGE) {
        jwt_free(jwt);
        return PLATFORM_TOKEN_INVALID;
    }

    puser.uid = uid;
    puser.platform = std::string(platform);
    puser.channel = std::string(channel);
    puser.sdk = std::string(sdk);
            
    //
    free(uid_str);
    jwt_free(jwt);
    return PLATFORM_OK;
}

} // namespace elf
