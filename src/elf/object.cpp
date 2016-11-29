/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/log.h>
#include <elf/object.h>
#include <elf/memory.h>

namespace elf {
obj_map_id Object::s_objs;
Object::proto_map Object::s_pbs;
id_lismap Object::s_containers;

Object::Object() :
    m_id(OID_NIL),
    m_sid(0),
    m_pid(OID_NIL),
    m_type(0),
    m_pb(NULL)
{
}

Object::Object(oid_t id)
    : m_id(id)
{
}

Object::~Object(void)
{
    DelPB(m_id, true);
    s_objs.erase(m_id);
}

void Object::OnInit(void)
{
    obj_map_id::const_iterator itr = s_objs.find(m_id);

    if (itr != s_objs.end()) {
        Object *obj = itr->second;

        LOG_ERROR("sys", "(%d)<%lld:%lld> DUPLICATED initialization (%s) - (%s)",
                m_type,
                m_pid,
                m_id,
                obj->Name().c_str(),
                m_name.c_str());
    }
    s_objs[m_id] = this;
    if (m_pb != NULL) {
        IndexProto(m_pb, m_pid, m_type, m_id);
    }
}

void Object::Stat(void)
{
    LOG_INFO("stat", "protobufs: %d, objects: %d",
            s_pbs.size(),
            s_objs.size());
}

void Object::Release(void)
{
    obj_map_id::iterator itr = s_objs.begin();
    for (; itr != s_objs.end(); ++itr) {
        Object *obj = itr->second;

        s_pbs.erase(obj->m_id);
        S_DELETE(obj);
    }

    proto_map::iterator itr_p = s_pbs.begin();
    for (; itr_p != s_pbs.end(); ++itr_p) {
        Proto *ref = itr_p->second;

        S_DELETE(ref->pb);
        S_DELETE(ref);
    }
}

Object *Object::FindObject(oid_t id)
{
    obj_map_id::const_iterator itr =s_objs.find(id);

    if (itr != s_objs.end()) {
        return itr->second;
    }
    return NULL;
}

pb_t *Object::FindPB(oid_t id)
{
    proto_map::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        return itr->second->pb;
    }
    return NULL;
}

bool Object::ClonePB(pb_t *pb, oid_t id)
{
    assert(pb);

    pb_t *src = FindPB(id);
    if (src == NULL) {
        return false;
    }
    pb->CopyFrom(*src);
    return true;
}

void Object::DelPB(oid_t id, bool recursive)
{
    UnindexProto(id, recursive);
}

Object::Proto *Object::FindProto(oid_t id) {
    proto_map::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        return itr->second;
    }
    return NULL;
}

void Object::IndexProto(pb_t *pb, oid_t pid, int type, oid_t id)
{
    Object::Proto *proto = FindProto(id);
    if (proto == NULL) {
        proto = E_NEW Proto;
        proto->pb = pb;
        proto->pid = pid;
        proto->type = type;
        proto->ref = 1;
        s_pbs[id] = proto;
        AddChild(pid, type, id);
        if (pid != elf::OID_NIL) {
            AddChild(elf::OID_NIL, type, id);
        }
    } else {
        proto->ref++;
    }
}

void Object::UnindexProto(oid_t id, bool recursive)
{
    Object::Proto *proto = FindProto(id);
    if (proto == NULL) {
        return;
    }
    oid_t pid = proto->pid;
    int type = proto->type;

    proto->ref--;
    if (proto->ref == 0) {
        proto->ref = 1;
        s_pbs.erase(id);
        DelChild(pid, type, id);
        if (pid != elf::OID_NIL) {
            DelChild(elf::OID_NIL, type, id);
        }
        E_DELETE(proto->pb);
        E_DELETE(proto);
        if (!recursive) {
            return;
        }

        id_lismap::iterator itr = s_containers.find(id);
        if (itr == s_containers.end()) {
            return;
        }

        id_ismap *ism = itr->second;
        if (ism == NULL) {
            return;
        }

        id_ismap::iterator itr_i = ism->begin();
        for (; itr_i != ism->end(); ++itr_i) {
            id_set *ctner = itr_i->second;

            if (ctner != NULL) {
                id_set::iterator itr_s = ctner->begin();
                for (; itr_s != ctner->end(); ++itr_s) {
                    UnindexProto(*itr_s, true);
                }
                S_DELETE(ctner);
            }
        }
        E_DELETE(ism);
        s_containers.erase(itr);
    }
}

id_set *Object::GetChildren(oid_t pid, int type)
{
    id_lismap::const_iterator itr = s_containers.find(pid);

    if (itr != s_containers.end()) {
        id_ismap *ism = itr->second;

        if (ism != NULL) {
            id_ismap::const_iterator itr_i = ism->find(type);

            if (itr_i != ism->end()) {
                return itr_i->second;
            }
        }
    }
    return NULL;
}

void Object::DelChildren(oid_t pid, int type)
{
    id_lismap::iterator itr = s_containers.find(pid);

    if (itr == s_containers.end()) {
        return;
    }
    id_ismap *ism = itr->second;

    if (ism == NULL) {
        return;
    }
    if (type <= 0) {
        id_ismap::iterator itr_i = ism->begin();
        for (; itr_i != ism->end(); ++itr_i) {
            S_DELETE(itr_i->second);
        }
        S_DELETE(ism);
        s_containers.erase(itr);
    } else {
        id_ismap::iterator itr_i = ism->find(type);

        if (itr_i != ism->end()) {
            S_DELETE(itr_i->second);
        }
        ism->erase(itr_i);
    }
}

oid_t Object::GetLastChild(oid_t pid, int type)
{
    id_set *is = GetChildren(pid, type);

    if (is != NULL) {
        id_set::const_reverse_iterator itr_s = is->rbegin();

        if (itr_s != is->rend()) {
            return *itr_s;
        }
    }
    return OID_NIL;
}

void Object::AddChild(oid_t pid, int type, oid_t id)
{
    oid_lismap_add(s_containers, pid, type, id);
}

void Object::SetChild(oid_t pid, int type, oid_t id)
{
    id_set *container = GetChildren(pid, type);

    if (container == NULL) {
        oid_lismap_add(s_containers, pid, type, id);
    } else {
        container->clear();
        container->insert(id);
    }
}

void Object::DelChild(oid_t pid, int type, oid_t id)
{
    oid_lismap_del(s_containers, pid, type, id);
}
} // namespace elf

