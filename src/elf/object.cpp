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
    m_id(0),
    m_uid(0),
    m_pid(0),
    m_type(0),
    m_idx(0),
    m_sid(0),
    m_pb(NULL)
{
}

Object::~Object(void)
{
    DelPB(m_id, false);
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
        IndexProto(m_pb, m_uid, m_pid, m_type, m_id, m_idx);
    }
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

oid_t Object::GetPID(oid_t id)
{
    Proto *proto = FindProto(id);

    if (proto != NULL) {
        return proto->pid;
    }
    return 0;
}

Proto *Object::FindProto(oid_t id) {
    proto_map::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        return itr->second;
    }
    return NULL;
}

pb_t *Object::FindPB(oid_t id, int type)
{
    proto_map::const_iterator itr =s_pbs.find(id);

    if (itr != s_pbs.end()) {
        Proto *proto = itr->second;

        if (type == 0 || proto->type == type) {
            return proto->pb;
        }
    }
    return NULL;
}

bool Object::ClonePB(pb_t *pb, oid_t id, int type)
{
    assert(pb);

    pb_t *src = FindPB(id, type);
    pb->CopyFrom(*src);
    return true;
}

void Object::DelPB(oid_t id, bool recursive)
{
    UnindexProto(id, recursive);
}

void Object::IndexProto(pb_t *pb, oid_t uid, oid_t pid, int type, oid_t id, int idx)
{
    Proto *parent = NULL;
    Proto *proto = FindProto(id);
    if (pid != 0) {
        parent = FindProto(pid);
    }
    if (proto == NULL) {
        proto = E_NEW Proto;
        proto->pb = pb;
        proto->id = id;
        proto->uid = uid;
        proto->idx = idx;
        proto->pid = pid;
        if (parent != NULL) {
            proto->ptype = parent->type;
        } else {
            proto->ptype = 0;
        }
        proto->type = type;
        proto->ref = 1;
        s_pbs[id] = proto;
        AddChild(pid, type, id);
        if (pid != 0) {
            AddChild(0, type, id);
        }
    } else {
        proto->ref++;
    }
}

void Object::UnindexProto(oid_t id, bool recursive)
{
    Proto *proto = FindProto(id);
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
        if (pid != 0) {
            DelChild(0, type, id);
        }
        E_DELETE(proto->pb);
        E_DELETE(proto);

        id_lismap::iterator itr = s_containers.find(id);
        if (itr == s_containers.end()) {
            return;
        }

        id_ismap *ism = itr->second;
        if (ism == NULL) {
            return;
        }

        if (recursive) {
            id_ismap::iterator itr_i = ism->begin();
            for (; itr_i != ism->end(); ++itr_i) {
                id_set ctner = *(itr_i->second); // erased while `UnindexProto`
                id_set::iterator itr_s = ctner.begin();
                for (; itr_s != ctner.end(); ++itr_s) {
                    UnindexProto(*itr_s, true);
                }

                S_DELETE(itr_i->second);
            }
            E_DELETE(ism);
            s_containers.erase(itr);
        }
    }
}

int Object::GetMaxType(oid_t pid)
{
    id_lismap::const_iterator itr = s_containers.find(pid);
    if (itr == s_containers.end()) {
        return 0;
    }

    id_ismap *ism = itr->second;
    if (ism == NULL || ism->empty()) {
        return 0;
    }
    id_ismap::const_reverse_iterator itr_i = ism->rbegin();
    return itr_i->first;
}

int Object::GetMaxIndex(oid_t pid, int type)
{
    oid_t cid = GetContainer(pid, type);

    return GetMaxType(cid);
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

oid_t Object::GetLastChild(oid_t pid, int type)
{
    id_set *is = GetChildren(pid, type);

    if (is != NULL) {
        id_set::const_reverse_iterator itr_s = is->rbegin();

        if (itr_s != is->rend()) {
            return *itr_s;
        }
    }
    return 0;
}

bool Object::HasChild(oid_t pid, int type, oid_t id)
{
    id_set *is = GetChildren(pid, type);

    if (is != NULL) {
        id_set::const_iterator itr = is->find(id);

        if (itr != is->end()) {
            return true;
        }
    }
    return false;
}

size_t Object::ChildrenSize(oid_t pid, int type)
{
    id_set *is = GetChildren(pid, type);

    if (is != NULL) {
        return is->size();
    }
    return 0;
}

oid_t Object::GetContainer(oid_t pid, int type)
{
    oid_t cid = GetLastChild(pid, type);

    if (cid == 0) {
        cid = oid_gen();
        SetChild(pid, type, cid);
    }
    return cid;
}

id_ismap *Object::GetContainerItems(oid_t pid, int type)
{
    oid_t cid = GetContainer(pid, type);
    id_lismap::const_iterator itr = s_containers.find(cid);

    if (itr != s_containers.end()) {
        return itr->second;
    }
    return NULL;
}

id_set *Object::GetContainerItems(oid_t pid, int type, int idx)
{
    oid_t cid = GetContainer(pid, type);

    return GetChildren(cid, idx);
}

pb_t *Object::GetContainerItem(oid_t pid, int type, int idx)
{
    oid_t cid = GetContainer(pid, type);
    oid_t oid = GetLastChild(cid, idx);
    pb_t *pb = NULL;

    if (oid != 0) {
        pb = FindPB(oid, 0);
        assert(pb);
    }
    return pb;
}

void Object::AddContainerItem(oid_t pid, int type, int idx, oid_t id)
{
    oid_t cid = GetContainer(pid, type);

    AddChild(cid, idx, id);
}

void Object::DelContainerItem(oid_t pid, int type, int idx, oid_t id)
{
    oid_t cid = GetContainer(pid, type);

    DelChild(cid, idx, id);
}

void Object::AddChild(oid_t pid, int type, oid_t id)
{
    oid_lismap_add(s_containers, pid, type, id);
}

void Object::SetChild(oid_t pid, int type, oid_t id)
{
    DelChildren(pid, type);
    AddChild(pid, type, id);
}

void Object::DelChild(oid_t pid, int type, oid_t id)
{
    oid_lismap_del(s_containers, pid, type, id);
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
    if (type == 0) {
        id_ismap::iterator itr_i = ism->begin();
        for (; itr_i != ism->end(); ++itr_i) {
            S_DELETE(itr_i->second);
        }
        LOG_TRACE("object", "%19lld - FREE",
                pid);
        S_DELETE(ism);
        s_containers.erase(itr);
    } else {
        id_ismap::iterator itr_i = ism->find(type);

        if (itr_i != ism->end()) {
            id_set *ids = itr_i->second;

            ids->clear();
            LOG_TRACE("object", "%19lld - %19s <%d>",
                    pid, "CLEAR", type);
        }
    }
}

bool Object::Stat(void *args)
{
    assert(args);
    time_t ct = *((time_t *)args);
    struct tm ctm;
    char cts[20];

    localtime_r(&ct, &ctm);
    strftime(cts, 20, "%F %T", &ctm);
    LOG_INFO("stat", "\n\n=========== %s ===========", cts);
    LOG_INFO("stat", "protobufs: %d, objects: %d, containers: %d\n",
            s_pbs.size(),
            s_objs.size(),
            s_containers.size());

    id_lismap::const_iterator itr = s_containers.find(0);
    if (itr == s_containers.end()) {
        return true;
    }

    id_ismap *ism = itr->second;
    id_ismap::const_iterator itr_a = ism->begin();
    for (; itr_a != ism->end(); ++itr_a) {
        size_t size = 0;
        elf::id_set *is = itr_a->second;
        if (is != NULL) {
            size = is->size();
            elf::id_set::const_iterator itr_i = is->begin();
            for (; itr_i != is->end(); ++itr_i) {
                Proto *proto = FindProto(*itr_i);
                if (proto != NULL) {
                    LOG_TRACE("stat", "%23lld - %19lld <%d:%d>",
                            proto->pid,
                            proto->id,
                            proto->idx,
                            proto->ref);
                } else {
                    LOG_TRACE("stat", "%23lld",
                            *itr_i);
                }
            }
        }
        LOG_INFO("stat", "--------- %13d (%d)\n",
                itr_a->first, size);
    }
    return true;
}
} // namespace elf

