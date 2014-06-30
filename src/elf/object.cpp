/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/object.h>
#include <elf/memory.h>

namespace elf {
obj_map_id Object::s_objs;

Object::Object()
    : m_id(OID_NIL),
    m_pb(NULL)
{
}

Object::Object(oid_t id)
    : m_id(id)
{
}

Object::~Object(void)
{
    S_DELETE(m_pb);
    s_objs.erase(m_id);
}

void Object::OnInit(void)
{
    obj_map_id::const_iterator itr = s_obj.find(m_id);

    if (itr != s_objs.end()) {
        Object *obj = itr->second;

        LOG_ERROR("sys", "<%lld>(%s) - (%s)",
                m_id,
                obj->GetName.c_str()(),
                m_name.c_str());
    }
    s_objs[m_id] = this;
}
} // namespace elf

