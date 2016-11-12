/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/json.h>
#include <elf/log.h>
#include <cJSON/cJSON.h>
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include <map>

using namespace google::protobuf;

namespace elf {
static cJSON *s_json;

bool json_bind(const char *path)
{
    assert(path);
    json_unbind(path);

    std::fstream fs(path, std::ios::in | std::ios::binary);

    if (!fs) {
        LOG_ERROR("json",
                "Can NOT open file %s.", path);
        return false;
    }

    std::stringstream iss;

    iss << fs.rdbuf();
    s_json = cJSON_Parse(iss.str().c_str());
    return true;
}

bool json_unbind(const char *path)
{
    cJSON_Delete(s_json);
    return true;
}

pb_t *json_pb(const char *pb_type, const char *json_type, const char *data)
{
    pb_t *pb = pb_create(pb_type);
 
    return json_pb(pb, json_type, data);
}

pb_t *json_pb(pb_t *pb, const char *json_type, const char *data)
{
    assert(pb);

    cJSON *json = cJSON_Parse(data);

    if (json == NULL) {
        return NULL;
    }

    const Descriptor *des = pb->GetDescriptor();
    cJSON *ref = cJSON_GetObjectItem(s_json, json_type);

    assert(ref);
    cJSON *item = json->child;
    for (; item; item = item->next) {
        if (item->type != cJSON_String) continue;
        if (strlen(item->valuestring) <= 0) continue;

        cJSON *ofd = cJSON_GetObjectItem(ref, item->string);

        if (!ofd) continue;

        const FieldDescriptor *fd = des->FindFieldByName(ofd->valuestring);

        if (!fd) continue;
        pb_set_field(pb, fd, item->valuestring);
    }
    cJSON_Delete(json);
    return pb;
}

pb_t *json_pb(const std::string &pb_type, const std::string &json)
{
    pb_t *pb = pb_create(pb_type);
    int st = json_pb(json, pb);

    if (st != 0) {
        return pb;
    }
    pb_destroy(pb);
    return NULL;
}

int json_pb(const std::string &json, pb_t *pb)
{
    assert(pb);

    util::Status st = google::protobuf::util::JsonStringToMessage(json, pb);

    return st.error_code();
}
} // namespace elf

