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
        s = E_NEW id_set;
        ism[idx] = s;
    } else {
        s = itr->second;
    }
    assert(s);
    s->insert(id);
}

void oid_ismap_del(id_ismap &ism, int idx, oid_t id)
{
    id_set *s = NULL;
    id_ismap::iterator itr = ism.find(idx);

    if (itr != ism.end()) {
        s = itr->second;
        s->erase(id);
    }
}

void oid_illmap_add(id_illmap &illm, int idx, oid_t k, oid_t v)
{
    id_llmap *m = NULL;
    id_illmap::iterator itr = illm.find(idx);

    if (itr == illm.end()) {
        m = E_NEW id_llmap;
        illm[idx] = m;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[k] = v;
}

void oid_illmap_del(id_illmap &illm, int idx, oid_t k)
{
    id_llmap *m = NULL;
    id_illmap::iterator itr = illm.find(idx);

    if (itr != illm.end()) {
        m = itr->second;
        m->erase(k);
    }
}
} // namespace elf

