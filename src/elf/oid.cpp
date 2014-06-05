/*
 * Copyright (C) 2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/oid.h>
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
} // namespace elf

