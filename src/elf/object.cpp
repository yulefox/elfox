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
    assert(s_objs.find(m_id) == s_objs.end());
    s_objs[m_id] = this;
}
} // namespace elf

