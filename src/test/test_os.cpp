/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/os.h>
#include <tut/tut.hpp>

namespace tut {
struct os {
    os() {
    }

    ~os() {
    }
};

typedef test_group<os> factory;
typedef factory::object object;

static tut::factory tf("os");

template<>
template<>
void object::test<1>() {
    set_test_name("mkdir");
    elf::os_mkdir("tmp");
    elf::os_mkdir("tmp");
}

template<>
template<>
void object::test<2>() {
    set_test_name("env");

    char val[20] = "GAME_=10";
    LOG_TEST("%s", elf::os_getenv("GAME_"));
    LOG_TEST("%d", elf::os_putenv(val));
    LOG_TEST("%s", elf::os_getenv("GAME_"));
    LOG_TEST("%d", elf::os_getenv_int("GAME_"));
}

template<>
template<>
void object::test<4>() {
}
}
