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
        LOG_ERROR("json", "load public key failed: %s", token);
        return PLATFORM_SETTING_ERROR;
    }
    json_t *app_id_node = json_object_get(s_platform_setting, "app_id");
    if (app_id_node == NULL) {
        LOG_ERROR("json", "get app id failed: %s", token);
        return PLATFORM_SETTING_ERROR;
    }

    const char *key_str = json_string_value(key);
    size_t key_len = json_string_length(key);
    if (key_str == NULL || key_len <= 0) {
        return PLATFORM_SETTING_ERROR;
    }

    ret = jwt_decode(&jwt, token, (const unsigned char*)key_str, (int)key_len);
    if (ret != 0) {
        LOG_ERROR("json", "jwt decode failed: %d %d %s", ret, errno, token);
        return PLATFORM_TOKEN_INVALID;
    }

    if (json_integer_value(app_id_node) != jwt_get_grant_int(jwt, "app_id")) {
        LOG_ERROR("json", "invalid app id: %s", token);
        jwt_free(jwt);
        return PLATFORM_TOKEN_INVALID;
    }

    if (time_s() > jwt_get_grant_int(jwt, "exp")) {
        LOG_ERROR("json", "token has expired: %s", token);
        jwt_free(jwt);
        return PLATFORM_TOKEN_EXPIRED;
    }

    char *uid_str = jwt_get_grants_json(jwt, "uid");
    const char *sid = jwt_get_grant(jwt, "sid");
    const char *platform = jwt_get_grant(jwt, "platform");
    const char *channel = jwt_get_grant(jwt, "channel");
    const char *sdk = jwt_get_grant(jwt, "sdk");
    const char *account = jwt_get_grant(jwt, "account");
    const char *reg_time = jwt_get_grant(jwt, "reg_time");
    const char *reg_time_s = jwt_get_grant(jwt, "reg_time_s");
    if (uid_str == NULL || platform == NULL || channel == NULL || sdk == NULL) {
        jwt_free(jwt);
        LOG_ERROR("json", "invalid token: %s", token);
        return PLATFORM_TOKEN_INVALID;
    }

    int64_t uid = strtoll(uid_str, NULL, 10);
    if (errno == EINVAL || errno == ERANGE) {
        jwt_free(jwt);
        LOG_ERROR("json", "parse uid failed: %s", token);
        return PLATFORM_TOKEN_INVALID;
    }

    puser.uid = uid;
    if (sid != NULL) {
        puser.sid = std::string(sid);
    }
    puser.platform = std::string(platform);
    puser.channel = std::string(channel);
    puser.sdk = std::string(sdk);
    if (account != NULL) {
        puser.account = std::string(account);
    }
    if (reg_time != NULL) {
        puser.reg_time = atoi(reg_time);
    }
    if (reg_time_s != NULL) {
        puser.reg_time_s = std::string(reg_time_s);
    }
    puser.status = jwt_get_grant_int(jwt, "status");

    //
    free(uid_str);
    jwt_free(jwt);
    return PLATFORM_OK;
}

} // namespace elf
