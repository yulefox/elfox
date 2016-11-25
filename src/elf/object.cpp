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
elf::id_lismap Object::s_containers;

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
    DelPB(m_id, m_pid, m_type);
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
        s_pbs[m_id] = m_pb;
        IndexProto(m_id, m_pid, m_type);
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
    while (itr != s_objs.end()) {
        Object *obj = itr->second;

        s_pbs.erase(obj->m_id);
        S_DELETE(obj);
    }

    proto_map::iterator itr_p = s_pbs.begin();
    while (itr_p != s_pbs.end()) {
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

pb_t *Object::AddPB(const pb_t &pb, oid_t id, oid_t pid, int type)
{
    pb_t *dst = FindPB(id);
    if (dst == NULL) {
        dst = E_NEW pb_t(pb);
        s_pbs[id] = dst;
    } else if (dst != &pb) {
        dst->CopyFrom(pb);
    }
    IndexProto(id, pid, type);
    return dst;
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

void Object::DelPB(oid_t id, oid_t pid, int type)
{
    pb_t *pb = FindPB(id);
    if (pb != NULL) {
        UnindexProto(id);
    }
}

Object::Proto *Object::FindProto(oid_t id) {
    proto_map::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        return itr->second;
    }
    return NULL;
}

void Object::IndexProto(pb_t *pb)
{
    assert(pb);
    Object::Proto *proto = FindProto(pb.id());
    if (proto == NULL) {
        proto = E_NEW Proto;

        proto->pb = m_pb;
        pr->ref = 1;
        s_pbs[m_id] = pr;
    }
}

void Object::UnindexProto(void)
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
        IndexProto(m_pb);

        Proto *pr = E_NEW Proto;

        pr->pb = m_pb;
        pr->ref = 1;
        s_pbs[m_id] = pr;
    }
}

elf::id_set *Object::GetChildren(elf::oid_t cid, int type)
{
    elf::id_lismap::const_iterator itr = s_containers.find(cid);

    if (itr != s_containers.end()) {
        elf::id_ismap *ism = itr->second;

        if (ism != NULL) {
            elf::id_ismap::const_iterator itr_i = ism->find(type);

            if (itr_i != ism->end()) {
                return itr_i->second;
            }
        }
    }
    return NULL;
}

void Object::DelChildren(elf::oid_t cid, int type)
{
    elf::id_lismap::iterator itr = s_containers.find(cid);

    if (itr == s_containers.end()) {
        return;
    }
    elf::id_ismap *ism = itr->second;

    if (ism == NULL) {
        return;
    }
    if (type <= 0) {
        elf::id_ismap::iterator itr_i = ism->begin();

        while (itr_i != ism->end()) {
            E_DELETE(itr_i->second);
            ism->erase(itr_i++);
        }
        E_DELETE(ism);
        s_containers.erase(itr);
    } else {
        elf::id_ismap::iterator itr_i = ism->find(type);

        if (itr_i != ism->end()) {
            E_DELETE(itr_i->second);
        }
        ism->erase(itr_i);
    }
}

elf::oid_t Object::GetLastChild(elf::oid_t cid, int type)
{
    elf::id_set *is = GetChildren(type, type);

    if (is != NULL) {
        elf::id_set::const_reverse_iterator itr_s = is->rbegin();

        if (itr_s != is->rend()) {
            return *itr_s;
        }
    }
    return elf::OID_NIL;
}

void Object::AddChild(elf::oid_t cid, int type, elf::oid_t oid)
{
    elf::oid_lismap_add(s_containers, cid, type, oid);
}

void Object::SetChild(elf::oid_t cid, int type, elf::oid_t oid)
{
    elf::id_set *container = GetChildren(cid, type);

    if (container == NULL) {
        elf::oid_lismap_add(s_containers, cid, type, oid);
    } else {
        container->clear();
        container->insert(oid);
    }
}

void Object::DelChild(elf::oid_t cid, int type, elf::oid_t oid)
{
    elf::oid_lismap_del(s_containers, cid, type, oid);
}
} // namespace elf

