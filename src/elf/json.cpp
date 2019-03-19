/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/json.h>
#include <elf/log.h>
#include <jansson.h>
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include <map>

using namespace google::protobuf;

namespace elf {
static json_t *s_json;

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
    s_json = json_loads(iss.str().c_str(), JSON_REJECT_DUPLICATES, NULL);
    return true;
}

bool json_unbind(const char *path)
{
    json_decref(s_json);
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

    json_t *json = json_loads(data, JSON_REJECT_DUPLICATES, NULL);

    if (json == NULL) {
        return NULL;
    }

    const Descriptor *des = pb->GetDescriptor();
    json_t *ref = json_object_get(s_json, json_type);    

    assert(ref);
    json_t *item;
    const char *key;
    json_object_foreach(json, key, item) {
        if (!json_is_string(item)) continue;
        if (json_string_length(item) <= 0) continue;

        json_t *ofd = json_object_get(ref, key);
        if (!ofd) {
            continue;
        }

        const FieldDescriptor *fd = des->FindFieldByName(json_string_value(ofd));
        if (!fd) continue;

        pb_set_field(pb, fd, json_string_value(item));
    }
    json_decref(json);
    return pb;
}

pb_t *json_pb(const std::string &pb_type, const std::string &json)
{
    pb_t *pb = pb_create(pb_type);
    int st = json2pb(json, pb);

    if (st != 0) {
        return pb;
    }
    pb_destroy(pb);
    return NULL;
}

int json2pb(const std::string &json, pb_t *pb, bool ignore_unknown_fields)
{
    assert(pb);

    google::protobuf::util::JsonParseOptions opts;
    opts.ignore_unknown_fields = ignore_unknown_fields;

    util::Status st = google::protobuf::util::JsonStringToMessage(json, pb, opts);
    if (st.error_code() != 0) {
        LOG_ERROR("json", "json2pb failed: %s %s", json.c_str(), st.ToString().c_str());
    }

    return st.error_code();
}

int pb2json(const pb_t &pb, std::string *json)
{
    assert(json);

    util::Status st = google::protobuf::util::MessageToJsonString(pb, json);

    return st.error_code();
}
} // namespace elf

