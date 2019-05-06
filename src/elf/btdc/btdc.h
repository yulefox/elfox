#ifndef __BT_DC_H
#define __BT_DC_H


#include <elf/config.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <string>
#include <deque>
#include <vector>
#include <map>

namespace elf {
class BTDC {

public:
static const int TOPIC_ODS_REGISTER                    = 3;
static const int TOPIC_ODS_LOGIN                       = 4;
static const int TOPIC_ODS_ONLINE_TIME                 = 5;
static const int TOPIC_ODS_ONLINE_USER_COUNT           = 16;
static const int TOPIC_ODS_EVENT_RECORD                = 17;
static const char *TOPIC_ODS_REGISTER_NAME;
static const char *TOPIC_ODS_LOGIN_NAME;
static const char *TOPIC_ODS_ONLINE_TIME_NAME;
static const char *TOPIC_ODS_ONLINE_USER_COUNT_NAME;
static const char *TOPIC_ODS_EVENT_RECORD_NAME;

struct Item {
    std::string key;
    std::string val;
};

class BaseTopic {
public:
    BaseTopic() {}
    BaseTopic(const BaseTopic &other) {
        id_ = other.ID();
        name_ = other.Name();
    }
    BaseTopic(int id, const char *name) {
        this->id_ = id;
        this->name_ = name;
    }
    virtual ~BaseTopic() {}

    int ID() const { return  id_; }
    std::string Name() const { return name_; }
    std::vector<Item> Args() const { return args_; }

    void Add(const std::string key, const std::string val) {
        Item item;
        item.key = key;
        item.val = val;
        args_.push_back(item);
    }

    void Add(const std::string key, int64_t val) {
        char buf[64];
        sprintf(buf, "%lld", val);
        Add(key, buf);
    }

    void Add(const std::string key, int val) {
        char buf[64];
        sprintf(buf, "%d", val);
        Add(key, buf);
    }

    void Add(const std::string key, float val) {
        char buf[64];
        sprintf(buf, "%0.2f", val);
        Add(key, buf);
    }

    bool Send() {
        time_t ct = time_s();
        struct tm ctm;
        char cts[20];
    
        localtime_r(&ct, &ctm);
        strftime(cts, 20, "%F %T", &ctm);

        Add("enterTime", cts);
        Add("logTime", cts);
        return BTDC::Send(this);
    }

protected:
    std::vector<Item> args_;
    std::string name_;
    int id_;
};

class RegisterTopic : public BaseTopic {
public:
    RegisterTopic() {
        BaseTopic(TOPIC_ODS_REGISTER, TOPIC_ODS_REGISTER_NAME);
    }
    virtual ~RegisterTopic() {}
};

class LoginTopic : public BaseTopic {
public:
    LoginTopic() {
        BaseTopic(TOPIC_ODS_LOGIN, TOPIC_ODS_LOGIN_NAME);
    }
    virtual ~LoginTopic() {}
};

class OnlineTimeTopic : public BaseTopic {
public:
    OnlineTimeTopic() {
        BaseTopic(TOPIC_ODS_ONLINE_TIME, TOPIC_ODS_ONLINE_TIME_NAME);
    }
    virtual ~OnlineTimeTopic() {}
};

class OnlineCountTopic : public BaseTopic {
public:
    OnlineCountTopic() {
        BaseTopic(TOPIC_ODS_ONLINE_USER_COUNT, TOPIC_ODS_ONLINE_USER_COUNT_NAME);
    }
    virtual ~OnlineCountTopic() {}
};

class EventTopic : public BaseTopic {
public:
    EventTopic(int event) {
        BaseTopic(TOPIC_ODS_EVENT_RECORD, TOPIC_ODS_EVENT_RECORD_NAME);
        Add("eventId", event);
    }
    virtual ~EventTopic() {}
};

static const int HTTP_POST_TIMEOUT = 5;

struct Event {
    BaseTopic topic;
    std::vector<Item> items;
    Event(const BaseTopic *topic) {
        this->topic = *topic;
        this->items = topic->Args();
    }
};

private:
    BTDC(const char *app_id, const char *push_url);
    void start();
    bool doSend(const std::string &raw);
    static void *sendLoop(void *data);

public:
    virtual ~BTDC();
public:
    static bool Init(const char *filename);
    static void Fini();
    static bool Send(const BaseTopic *topic);

private:
    static BTDC *inst;

private:
    xqueue<Event> wque_;
    std::string app_id_;
    std::string push_url_;
    thread_t worker_;
};

} // namespace elf


#endif /* !__BT_DC_H */
