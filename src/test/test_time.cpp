/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/time.h>
#include <tut/tut.hpp>

namespace tut {
struct time {
    time() {
    }

    ~time() {
    }
};

typedef test_group<time> factory;
typedef factory::object object;

static tut::factory tf("time");

template<>
template<>
void object::test<1>() {
    set_test_name("current time");
    LOG_TEST("%lld:%lld",
            elf::time_s(),
            elf::time_ms());
    ensure(elf::time_ms() > 0);
}

template<>
template<>
void object::test<2>() {
    set_test_name("delta time");
    elf::time64_t t1 = elf::time_ms();

    sleep(1);

    elf::time64_t t2 = elf::time_ms();
    ensure(elf::time_diff(t2, t1) == 1000);
}
}
