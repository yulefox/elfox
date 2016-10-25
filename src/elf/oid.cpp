/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/oid.h>
#include <elf/memory.h>
#include <elf/time.h>

namespace elf {
int MAGIC_INDEX = 0;
const int MAX_INDEX  = 920000ll;
const oid_t MAX_TIME = 10000000000000ll;

oid_t oid_gen(void)
{
    static oid_t id = 0;
    oid_t time = (oid_t)time_ms();

    assert(MAGIC_INDEX < MAX_INDEX && time < MAX_TIME);

    oid_t cid = ((oid_t)MAGIC_INDEX * MAX_TIME) + time;

    if (id >= cid) {
        ++id;
    } else {
        id = cid;
    }
    return id;
}

void oid_ismap_add(id_ismap &ism, int idx, oid_t id)
{
    id_set *s = NULL;
    id_ismap::iterator itr = ism.find(idx);

    if (itr == ism.end()) {
        ism[idx] = s = E_NEW id_set;
    } else {
        s = itr->second;
    }
    assert(s);
    s->insert(id);
}

void oid_ismap_del(id_ismap &ism, int idx, oid_t id)
{
    id_ismap::iterator itr = ism.find(idx);

    if (itr != ism.end()) {
        id_set *s = itr->second;

        s->erase(id);
    }
}

void oid_lsmap_add(id_lsmap &lsm, oid_t idx, oid_t id)
{
    id_set *s = NULL;
    id_lsmap::iterator itr = lsm.find(idx);

    if (itr == lsm.end()) {
        lsm[idx] = s = E_NEW id_set;
    } else {
        s = itr->second;
    }
    assert(s);
    s->insert(id);
}

void oid_lsmap_del(id_lsmap &lsm, oid_t idx, oid_t id)
{
    id_lsmap::iterator itr = lsm.find(idx);

    if (itr != lsm.end()) {
        id_set *s = itr->second;

        s->erase(id);
    }
}

void oid_illmap_add(id_illmap &illm, int idx, oid_t k, oid_t v)
{
    id_llmap *m = NULL;
    id_illmap::iterator itr = illm.find(idx);

    if (itr == illm.end()) {
        illm[idx] = m = E_NEW id_llmap;;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[k] = v;
}

void oid_illmap_del(id_illmap &illm, int idx, oid_t k)
{
    id_illmap::iterator itr = illm.find(idx);

    if (itr != illm.end()) {
        id_llmap *m = itr->second;

        m->erase(k);
    }
}

void oid_iismap_add(id_iismap &iism, int idx, int k, oid_t v)
{
    id_ismap *ism = NULL;
    id_iismap::iterator itr = iism.find(idx);

    if (itr == iism.end()) {
        iism[idx] = ism = E_NEW id_ismap;
    } else {
        ism = itr->second;
    }

    id_ismap::iterator itr_i = ism->find(k);
    id_set *is = NULL;

    if (itr_i == ism->end()) {
        (*ism)[k] = is = E_NEW id_set;
    } else {
        is = itr_i->second;
    }
    assert(is);
    is->insert(v);
}

void oid_iismap_del(id_iismap &iism, int idx, int k, oid_t v)
{
    id_iismap::iterator itr = iism.find(idx);

    if (itr != iism.end()) {
        id_ismap *ism = itr->second;

        id_ismap::iterator itr_i = ism->find(k);

        if (itr_i != ism->end()) {
            id_set *is = itr_i->second;

            assert(is);
            is->erase(v);
        }
    }
}
} // namespace elf

