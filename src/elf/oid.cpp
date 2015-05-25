/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/oid.h>
#include <elf/memory.h>
#include <elf/time.h>

namespace elf {
int MAGIC_INDEX = 0;
const int MAX_INDEX = 100000ll;
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
void oid_ipmap_add(id_ipmap &mm, int idx, oid_t id_1, oid_t id_2)
{
    id_pmap *m = NULL;
    id_ipmap::iterator itr = mm.find(idx);

    if (itr == mm.end()) {
        m = E_NEW id_pmap;
        mm[idx] = m;
    } else {
        m = itr->second;
    }
    assert(m);
    (*m)[id_1] = id_2;
}
} // namespace elf

