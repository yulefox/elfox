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
#include <sstream>
#include <map>
#include <deque>

namespace elf {

    static std::string itos(int val)
    {
        std::string s;
        std::stringstream ss(s);
        ss >> val;
        return s;
    }
    static std::string itos(int64_t val)
    {
        std::string s;
        std::stringstream ss(s);
        ss >> val;
        return s;
    }

    static size_t cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
        return size * nmemb;
    }

    int qg_stat_login(const std::string &userId, int server, int loginTime)
    {
        cJSON *setting = platform_get_json(PLAT_QIANGUI);
        if (setting == NULL) {
            return PLATFORM_SETTING_ERROR;
        }
        
        cJSON *url = cJSON_GetObjectItem(setting, "URL");
        if (url == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appId = NULL;
        if (server < 200000) {
            appId = cJSON_GetObjectItem(setting, "appIdIOS");
        } else {
            appId = cJSON_GetObjectItem(setting, "appId");
        }
        if (appId == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
        if (appKey == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        std::string sign;
        sign.append(appId->valuestring);
        sign.append(itos(loginTime));
        sign.append(itos(server));
        sign.append(userId);
        sign.append(appKey->valuestring);
        sign = md5((unsigned char*)sign.c_str(), sign.length());

        ///
        std::string post_url;
        post_url.append(url->valuestring);
        post_url.append("/logLogin");
        post_url.append("?gameId=");
        post_url.append(appId->valuestring);

        post_url.append("&loginTime=");
        post_url.append(itos(loginTime));

        post_url.append("&serverNo=");
        post_url.append(itos(server));

        post_url.append("&userId=");
        post_url.append(userId);

        post_url.append("&sign=");
        post_url.append(sign);

        // do post request
        http_json(HTTP_POST, post_url.c_str(), "", cb, NULL);
        return 0;
    }

    int qg_stat_create(const std::string &userId, int server, int64_t roleId, int createTime)
    {
        cJSON *setting = platform_get_json(PLAT_QIANGUI);
        if (setting == NULL) {
            return PLATFORM_SETTING_ERROR;
        }
        
        cJSON *url = cJSON_GetObjectItem(setting, "URL");
        if (url == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appId = NULL;
        if (server < 200000) {
            appId = cJSON_GetObjectItem(setting, "appIdIOS");
        } else {
            appId = cJSON_GetObjectItem(setting, "appId");
        }
        if (appId == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
        if (appKey == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        std::string sign;
        sign.append(itos(createTime));
        sign.append(appId->valuestring);
        sign.append(itos(roleId));
        sign.append(itos(server));
        sign.append(userId);
        sign.append(appKey->valuestring);
        sign = md5((unsigned char*)sign.c_str(), sign.length());

        ///http://www.qg8.com/7yworld-ad-collection/gameStatistics/createRole?gameId=游戏ID&createTime=创建时间&serverNo=区服编号&userId=用户ID&roleId=角色ID&sign=签名
        ///
        std::string post_url;
        post_url.append(url->valuestring);
        post_url.append("/createRole");
        post_url.append("?gameId=");
        post_url.append(appId->valuestring);

        post_url.append("&createTime=");
        post_url.append(itos(createTime));

        post_url.append("&serverNo=");
        post_url.append(itos(server));

        post_url.append("&userId=");
        post_url.append(userId);

        post_url.append("&roleId=");
        post_url.append(itos(roleId));

        post_url.append("&sign=");
        post_url.append(sign);

        // do post request
        http_json(HTTP_POST, post_url.c_str(), "", cb, NULL);
        return 0;
    }


    int qg_stat_online_5m(int server, int total, int time)
    {
        cJSON *setting = platform_get_json(PLAT_QIANGUI);
        if (setting == NULL) {
            return PLATFORM_SETTING_ERROR;
        }
        
        cJSON *url = cJSON_GetObjectItem(setting, "URL");
        if (url == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appId = NULL;
        if (server < 200000) {
            appId = cJSON_GetObjectItem(setting, "appIdIOS");
        } else {
            appId = cJSON_GetObjectItem(setting, "appId");
        }
        if (appId == NULL) {
            return PLATFORM_SETTING_ERROR;
        }


        cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
        if (appKey == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        std::string sign;
        sign.append(itos(time));
        sign.append(appId->valuestring);
        sign.append(itos(total));
        sign.append(itos(server));
        sign.append(appKey->valuestring);
        sign = md5((unsigned char*)sign.c_str(), sign.length());

        ///
        std::string post_url;
        post_url.append(url->valuestring);
        post_url.append("/online/5m");
        post_url.append("?gameId=");
        post_url.append(appId->valuestring);

        post_url.append("&createTime=");
        post_url.append(itos(time));

        post_url.append("&serverNo=");
        post_url.append(itos(server));

        post_url.append("&onlineNum=");
        post_url.append(itos(total));

        post_url.append("&sign=");
        post_url.append(sign);

        // do post request
        http_json(HTTP_POST, post_url.c_str(), "", cb, NULL);

        return 0;
    }

    /*
     *参数名称	类型	是否必须	描述	示例
userId	long	必须	钱柜游戏平台用户ID	如：1
gameId	int	必须	游戏ID	如：1
serverNo	int	必须	游戏区服ID	如：1
roleId	int	必须	CP方玩家角色ID	如：1
roleName	varchar(50)	必须	角色名称	如：烟雨江南
roleLevel	int	必须	玩家等级	如：52
roleCareer	varchar(10)	必须	玩家职业	如：战士
roleFightPower	long	必须	玩家战力值	如：9999
offlineTime	long	必须	签名时间(单位毫秒)，供CP判断参数有效性时间	如：1460449914
sign	varchar(32)	必须	md5(gameId+offlineTime+roleCareer+roleFightPower+roleId+roleLevel+
roleName+serverNo+userId+key) 
md5字符串为小写字母
注意：加密串参数的顺序是按照字母顺序排列的，顺序错误会导致签名不正确
Key不参与排序，放在最后，切记！
key是表示平台和游戏双方提前协商约定好的密钥
*/
    int qg_stat_logout(const std::string &userId, int server,
            int64_t roleId, const std::string &roleName,
            int roleLevel, int roleCareer, int roleFightPower, int offlineTime)
    {
        cJSON *setting = platform_get_json(PLAT_QIANGUI);
        if (setting == NULL) {
            return PLATFORM_SETTING_ERROR;
        }
        
        cJSON *url = cJSON_GetObjectItem(setting, "URL");
        if (url == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        cJSON *appId = NULL;
        if (server < 200000) {
            appId = cJSON_GetObjectItem(setting, "appIdIOS");
        } else {
            appId = cJSON_GetObjectItem(setting, "appId");
        }
        if (appId == NULL) {
            return PLATFORM_SETTING_ERROR;
        }


        cJSON *appKey = cJSON_GetObjectItem(setting, "appKey");
        if (appKey == NULL) {
            return PLATFORM_SETTING_ERROR;
        }

        //md5(gameId+offlineTime+roleCareer+roleFightPower+roleId+roleLevel+roleName+serverNo+userId+key) 

        std::string sign;
        sign.append(appId->valuestring);
        sign.append(itos(offlineTime));
        sign.append(itos(roleCareer));
        sign.append(itos(roleFightPower));
        sign.append(itos(roleId));
        sign.append(itos(roleLevel));
        sign.append(roleName);
        sign.append(itos(server));
        sign.append(userId);
        sign.append(appKey->valuestring);
        sign = md5((unsigned char*)sign.c_str(), sign.length());

        //http://localhost:8080/7yworld-ad-collection/gameStatistics/logLogout?gameId=游戏ID&offlineTime=离线时间&roleCareer=职业&roleFightPower=战力&roleId=角色ID&roleLevel=角色等级&roleName=角色名称&serverNo=区服编号&userId=用户ID&sign=签名
        ///
        std::string post_url;
        post_url.append(url->valuestring);
        post_url.append("/logLogout");
        post_url.append("?gameId=");
        post_url.append(appId->valuestring);

        post_url.append("&offlineTime=");
        post_url.append(itos(offlineTime));

        post_url.append("&roleCareer=");
        post_url.append(itos(roleCareer));

        post_url.append("&roleFightPower=");
        post_url.append(itos(roleFightPower));

        post_url.append("&roleId=");
        post_url.append(itos(roleId));

        post_url.append("&roleLevel=");
        post_url.append(itos(roleLevel));

        post_url.append("&roleLevel=");
        post_url.append(itos(roleLevel));

        post_url.append("&roleName=");
        post_url.append(roleName);

        post_url.append("&serverNo=");
        post_url.append(itos(server));

        post_url.append("&userId=");
        post_url.append(userId);

        post_url.append("&sign=");
        post_url.append(sign);


        // do post request
        http_json(HTTP_POST, post_url.c_str(), "", cb, NULL);
        return 0;
    }


}
