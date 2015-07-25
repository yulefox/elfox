/*
 * Copyright (C) 2013-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/pb.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <map>
#include <string>

using namespace google::protobuf;

namespace elf {
typedef std::map<std::string, pb_new> reg_map;

static reg_map s_regs;

void pb_regist(const std::string &name, pb_new init)
{
    s_regs[name] = init;
}

pb_t *pb_create(const std::string &name)
{
    reg_map::const_iterator itr = s_regs.find(name);

    if (itr != s_regs.end()) {
        pb_new init = itr->second;

        return init();
    }
    return NULL;
}

void pb_destroy(pb_t *pb)
{
    E_DELETE pb;
}

int pb_get_int(const pb_t &pb, int num)
{
    const Descriptor *des = pb.GetDescriptor();
    const Reflection *ref = pb.GetReflection();
    const FieldDescriptor *fd = des->FindFieldByNumber(num);

    return ref->GetInt32(pb, fd);
}

pb_t *pb_get_field(pb_t *pb, const std::string &key)
{
    assert(pb);
    const Descriptor *des = pb->GetDescriptor();
    const Reflection *ref = pb->GetReflection();
    const FieldDescriptor *fd = des->FindFieldByName(key);

    return ref->MutableMessage(pb, fd);
}

void pb_set_field(pb_t *pb, const std::string &key,
        const char *val)
{
    assert(pb && val);
    const Descriptor *des = pb->GetDescriptor();
    const FieldDescriptor *fd = des->FindFieldByName(key);

    pb_set_field(pb, fd, val);
}

void pb_set_field(pb_t *pb, const FieldDescriptor *fd,
        const char *val)
{
    assert(pb && fd && val);

    const Reflection *ref = pb->GetReflection();
    FieldDescriptor::CppType type = fd->cpp_type();

    switch (type) {
        case FieldDescriptor::CPPTYPE_INT32:
            ref->SetInt32(pb, fd, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_INT64:
            ref->SetInt64(pb, fd, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT32:
            ref->SetUInt32(pb, fd, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT64:
            ref->SetUInt64(pb, fd, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_FLOAT:
            ref->SetFloat(pb, fd, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
            ref->SetDouble(pb, fd, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_BOOL:
            ref->SetBool(pb, fd, (atoi(val) > 0) ? true : false);
            break;
        case FieldDescriptor::CPPTYPE_STRING:
            ref->SetString(pb, fd, val);
            break;
        default:
            LOG_ERROR("pb",
                    "Invalid field type %d.",
                    type);
            assert(0);
    }
}

void pb_set_field(pb_t *pb, const FieldDescriptor *fd,
        const char *val, int len)
{
    assert(pb && fd && val);

    const Reflection *ref = pb->GetReflection();
    FieldDescriptor::CppType type = fd->cpp_type();

    switch (type) {
        case FieldDescriptor::CPPTYPE_INT32:
            ref->SetInt32(pb, fd, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_INT64:
            ref->SetInt64(pb, fd, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT32:
            ref->SetUInt32(pb, fd, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT64:
            ref->SetUInt64(pb, fd, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_FLOAT:
            ref->SetFloat(pb, fd, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
            ref->SetDouble(pb, fd, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_BOOL:
            ref->SetBool(pb, fd, (atoi(val) > 0) ? true : false);
            break;
        case FieldDescriptor::CPPTYPE_STRING:
            ref->SetString(pb, fd, std::string(val, len));
            break;
        default:
            LOG_ERROR("pb",
                    "Invalid field type %d.",
                    type);
            assert(0);
    }
}
} // namespace elf

