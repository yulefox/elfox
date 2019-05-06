#include <elf/elf.h>
#include <elf/log.h>
#include <elf/btdc/btdc.h>
#include <elf/net/http.h>
#include <jansson.h>

namespace elf {

BTDC *BTDC::inst = NULL;

const char *BTDC::TOPIC_ODS_REGISTER_NAME             = "ods_register";
const char *BTDC::TOPIC_ODS_LOGIN_NAME                = "ods_login";
const char *BTDC::TOPIC_ODS_ONLINE_TIME_NAME          = "ods_online_time";
const char *BTDC::TOPIC_ODS_ONLINE_USER_COUNT_NAME    = "ods_online_user_count";
const char *BTDC::TOPIC_ODS_EVENT_RECORD_NAME         = "ods_event_record";

BTDC::BTDC(const char *app_id, const char *push_url)
{
    app_id_ = app_id;
    push_url_ = push_url;
}

BTDC::~BTDC()
{
}

void BTDC::start()
{
    worker_ = thread_init(sendLoop, this);
}

bool BTDC::Init(const char *filename)
{
    if (inst != NULL) {
        LOG_ERROR("btdc", "%s", "BTDC has already been initialized.");
        return false;
    }

    json_error_t error;
    json_t *cfg = json_load_file(filename, 0, &error);
    if (cfg == NULL) {
        LOG_ERROR("btdc", "load config %s, failed: %s.", filename, error.text);
        return false;
    }

    json_t *mode_cfg = json_object_get(cfg, "mode");
    if (mode_cfg == NULL) {
        LOG_ERROR("btdc", "no found mode in file: %s", filename);
        json_decref(cfg);
        return false;
    }

    json_t *app_id_cfg = json_object_get(cfg, "app_id");
    if (app_id_cfg == NULL || json_string_value(app_id_cfg) == NULL) {
        LOG_ERROR("btdc", "no found app_id in file: %s", filename);
        json_decref(cfg);
        return false;
    }

    json_t *push_url_cfg = json_object_get(cfg, "push_url");
    if (push_url_cfg == NULL) {
        LOG_ERROR("btdc", "no found push_url in file: %s", filename);
        json_decref(cfg);
        return false;
    }

    json_t *push_url = NULL;
    const char *mode = json_string_value(mode_cfg);
    if (mode == NULL || !strcmp(mode, "debug")) {
        push_url = json_object_get(push_url_cfg, "debug");
    } else {
        push_url = json_object_get(push_url_cfg, "prod");
    }
    if (push_url == NULL || json_string_value(push_url) == NULL) {
        LOG_ERROR("btdc", "no found push_url in file: %s", filename);
        json_decref(cfg);
        return false;
    }
    inst = E_NEW BTDC(json_string_value(app_id_cfg), json_string_value(push_url));
    inst->start();
    json_decref(cfg);
    return true;
}

void BTDC::Fini()
{
    if (inst != NULL) {
        E_DELETE inst;
    }
}

bool BTDC::Send(const BaseTopic *topic)
{
    if (inst == NULL) {
        LOG_ERROR("btdc", "%s", "btdc instance was not been initialized.");
        return false;
    }
    if (topic == NULL) {
        LOG_ERROR("btdc", "%s", "null topic ptr.");
        return false;
    }
    Event ev = Event(topic);
    inst->wque_.push(ev);
    return true;
}

static bool buildJSON(const BTDC::Event &event, std::string &output)
{
    json_t *ctx = json_pack("{siss}", "topicId", event.topic.ID(), "topicName", event.topic.Name().c_str());
    if (ctx == NULL) {
        LOG_ERROR("btdc", "%s", "build json object failed");
        return false;
    }
    for (size_t i = 0; i < event.items.size(); i++) {
        const BTDC::Item &item = event.items[i];
        json_t *val = json_string(item.val.c_str());
        if (val == NULL) {
            json_decref(val);
            return false;
        }
        if (json_object_set(ctx, item.key.c_str(), val) < 0) {
            json_decref(val);
            return false;
        }
        json_decref(val);
    }
    char *raw = json_dumps(ctx, 0);
    if (raw == NULL) {
        json_decref(ctx);
        return false;
    }
    output.append(raw);
    free(raw);
    json_decref(ctx);
    return true;
}

void *BTDC::sendLoop(void *data)
{
    BTDC *inst = static_cast<BTDC*>(data);
    if (inst == NULL) {
        LOG_ERROR("btdc", "%s", "invalid BTDC inst.");
        return NULL;
    }
    std::deque<std::string> pending;
    while (true) {
        std::deque<Event> tmp;
        std::deque<Event>::iterator itr;
        inst->wque_.swap(tmp);
        for (itr = tmp.begin(); itr != tmp.end(); ++itr) {
            const Event &event = *itr;
            std::string raw;
            if (!buildJSON(event, raw)) {
                LOG_ERROR("btdc", "build json failed:: %d, %s", event.topic.ID(), event.topic.Name().c_str());
            }
            pending.push_back(raw);
        }

        std::deque<std::string> failed;
        while (!pending.empty()) {
            const std::string raw = pending.front();
            pending.pop_front();
            if (!inst->doSend(raw)) {
                failed.push_back(raw);
            }
        }
        if (!failed.empty()) {
            pending.swap(failed);
        }
        usleep(500000);
    }
    return NULL;
}

bool BTDC::doSend(const std::string &raw)
{
    std::string res;
    int ret = http_json(push_url_.c_str(), raw.c_str(), res, HTTP_POST_TIMEOUT);
    if (ret < 0) {
        LOG_INFO("btdc", "try to send: %s %s failed", push_url_.c_str(), raw.c_str());
        return false;
    }
    LOG_INFO("btdc", "try to send: %s %s, res=%s", push_url_.c_str(), raw.c_str(), res.c_str());
    return res.compare("success") == 0;
}

}
