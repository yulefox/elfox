/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/oid.h>
#include <elf/memory.h>
#include <elf/time.h>

namespace elf {
int MAGIC_INDEX = 0;
static const int MAX_INDEX  = 920000ll;
static const oid_t MAX_TIME = 10000000000000ll;

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

void oid_ismap_add(id_ismap &ism, int key, oid_t val)
{
    id_set *s = NULL;
    id_ismap::iterator itr = ism.find(key);

    if (itr == ism.end()) {
        ism[key] = s = E_NEW id_set;
    } else {
        s = itr->second;
    }
    assert(s);
    s->insert(val);
}

void oid_ismap_del(id_ismap &ism, int key, oid_t val)
{
    id_ismap::iterator itr = ism.find(key);

    if (itr != ism.end()) {
        id_set *s = itr->second;

        s->erase(val);
    }
}

void oid_lsmap_add(id_lsmap &lsm, oid_t key, oid_t val)
{
    id_set *s = NULL;
    id_lsmap::iterator itr = lsm.find(key);

    if (itr == lsm.end()) {
        lsm[key] = s = E_NEW id_set;
    } else {
        s = itr->second;
    }
    assert(s);
    s->insert(val);
}

void oid_lsmap_del(id_lsmap &lsm, oid_t key, oid_t val)
{
    id_lsmap::iterator itr = lsm.find(key);

    if (itr != lsm.end()) {
        id_set *s = itr->second;

        s->erase(val);
    }
}

void oid_illmap_add(id_illmap &illm, int key, oid_t k, oid_t v)
{
    id_llmap *m = NULL;
    id_illmap::iterator itr = illm.find(key);

    if (itr == illm.end()) {
        illm[key] = m = E_NEW id_llmap;;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[k] = v;
}

void oid_illmap_del(id_illmap &illm, int key, oid_t k)
{
    id_illmap::iterator itr = illm.find(key);

    if (itr != illm.end()) {
        id_llmap *m = itr->second;

        m->erase(k);
    }
}

void oid_iismap_add(id_iismap &iism, int key, int k, oid_t v)
{
    id_ismap *ism = NULL;
    id_iismap::iterator itr = iism.find(key);

    if (itr == iism.end()) {
        iism[key] = ism = E_NEW id_ismap;
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

void oid_iismap_del(id_iismap &iism, int key, int k, oid_t v)
{
    id_iismap::iterator itr = iism.find(key);

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

void oid_ilsmap_add(id_ilsmap &ilsm, int key, int k, oid_t v)
{
    id_lsmap *ism = NULL;
    id_ilsmap::iterator itr = ilsm.find(key);

    if (itr == ilsm.end()) {
        ilsm[key] = ism = E_NEW id_lsmap;
    } else {
        ism = itr->second;
    }

    id_lsmap::iterator itr_i = ism->find(k);
    id_set *is = NULL;

    if (itr_i == ism->end()) {
        (*ism)[k] = is = E_NEW id_set;
    } else {
        is = itr_i->second;
    }
    assert(is);
    is->insert(v);
}

void oid_ilsmap_del(id_ilsmap &ilsm, int key, int k, oid_t v)
{
    id_ilsmap::iterator itr = ilsm.find(key);

    if (itr != ilsm.end()) {
        id_lsmap *ism = itr->second;

        id_lsmap::iterator itr_i = ism->find(k);

        if (itr_i != ism->end()) {
            id_set *is = itr_i->second;

            assert(is);
            is->erase(v);
        }
    }
}

void oid_lilmap_add(id_lilmap &lilm, oid_t key, int k, oid_t v)
{
    id_ilmap *m = NULL;
    id_lilmap::iterator itr = lilm.find(key);

    if (itr == lilm.end()) {
        lilm[key] = m = E_NEW id_ilmap;;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[k] = v;
}

void oid_lilmap_del(id_lilmap &lilm, oid_t key, int k)
{
    id_lilmap::iterator itr = lilm.find(key);

    if (itr != lilm.end()) {
        id_ilmap *m = itr->second;

        m->erase(k);
    }
}

void oid_lllmap_add(id_lllmap &lllm, oid_t key, oid_t k, oid_t v)
{
    id_llmap *m = NULL;
    id_lllmap::iterator itr = lllm.find(key);

    if (itr == lllm.end()) {
        lllm[key] = m = E_NEW id_llmap;;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[k] = v;
}

void oid_lllmap_del(id_lllmap &lllm, oid_t key, oid_t k)
{
    id_lllmap::iterator itr = lllm.find(key);

    if (itr != lllm.end()) {
        id_llmap *m = itr->second;

        m->erase(k);
    }
}

void oid_lismap_add(id_lismap &lism, oid_t key, int k, oid_t v)
{
    id_ismap *ism = NULL;
    id_lismap::iterator itr = lism.find(key);

    if (itr == lism.end()) {
        lism[key] = ism = E_NEW id_ismap;
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

void oid_lismap_del(id_lismap &lism, oid_t key, int k, oid_t v)
{
    id_lismap::iterator itr = lism.find(key);

    if (itr != lism.end()) {
        id_ismap *ism = itr->second;

        id_ismap::iterator itr_i = ism->find(k);

        if (itr_i != ism->end()) {
            id_set *is = itr_i->second;

            assert(is);
            is->erase(v);
        }
    }
}

void oid_llsmap_add(id_llsmap &llsm, oid_t key, int k, oid_t v)
{
    id_lsmap *ism = NULL;
    id_llsmap::iterator itr = llsm.find(key);

    if (itr == llsm.end()) {
        llsm[key] = ism = E_NEW id_lsmap;
    } else {
        ism = itr->second;
    }

    id_lsmap::iterator itr_i = ism->find(k);
    id_set *is = NULL;

    if (itr_i == ism->end()) {
        (*ism)[k] = is = E_NEW id_set;
    } else {
        is = itr_i->second;
    }
    assert(is);
    is->insert(v);
}

void oid_llsmap_del(id_llsmap &llsm, oid_t key, int k, oid_t v)
{
    id_llsmap::iterator itr = llsm.find(key);

    if (itr != llsm.end()) {
        id_lsmap *ism = itr->second;

        id_lsmap::iterator itr_i = ism->find(k);

        if (itr_i != ism->end()) {
            id_set *is = itr_i->second;

            assert(is);
            is->erase(v);
        }
    }
}
} // namespace elf

