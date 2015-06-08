/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/log.h>
#include <elf/object.h>
#include <elf/memory.h>

namespace elf {
obj_map_id Object::s_objs;
Object::pbref_map_id Object::s_pbs;

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
    DelPB(m_id);
    s_objs.erase(m_id);
}

void Object::OnInit(void)
{
    obj_map_id::const_iterator itr = s_objs.find(m_id);

    if (itr != s_objs.end()) {
        Object *obj = itr->second;

        LOG_ERROR("sys", "<%lld>(%s) - (%s)",
                m_id,
                obj->GetName().c_str(),
                m_name.c_str());
    }
    s_objs[m_id] = this;
    if (m_pb != NULL) {
        PBRef *pr = E_NEW PBRef;

        pr->pb = m_pb;
        pr->ref = 1;
        s_pbs[m_id] = pr;
    }
}

void Object::Release(void)
{
    obj_map_id::iterator itr = s_objs.begin();
    while (itr != s_objs.end()) {
        Object *obj = itr->second;

        s_pbs.erase(obj->m_id);
        S_DELETE(obj);
    }

    pbref_map_id::iterator itr_p = s_pbs.begin();
    while (itr_p != s_pbs.end()) {
        PBRef *ref = itr_p->second;

        S_DELETE(ref->pb);
        S_DELETE(ref);
    }
}

void Object::DelPB(oid_t id)
{
    pbref_map_id::const_iterator itr = s_pbs.find(id);

    if (itr != s_pbs.end()) {
        PBRef *pr = itr->second;

        --(pr->ref);
        if (pr->ref <= 0) {
            E_DELETE(pr->pb);
            E_DELETE(pr);
            s_pbs.erase(id);
        }
    }
}

Object::PBRef *Object::FindRef(oid_t id) {
    pbref_map_id::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        return itr->second;
    }
    return NULL;
}
} // namespace elf

