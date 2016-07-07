/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/script/script.h>
#include <elf/time.h>
#include <tut/tut.hpp>
#include <bind/bind.hpp>

using namespace elf;

namespace tut {
struct script {
    script() {
    }

    ~script() {
    }
};

typedef test_group<script> factory;
typedef factory::object object;

tut::factory tf("script");

template<>
template<>
void object::test<1>() {
    set_test_name("Run File");
    tolua_bind_open(elf::script_get_state());

    int rc = script_file_exec("scripts/init.lua");

    if (rc != 0) {
        printf("%d", rc);
    }
    // truncated 64-bit time64_t into 32-bit integer
    rc = script_func_exec("S.test", "i", (int)elf::time_ms());

    int count = 20;
    while (--count) {
        // truncated 64-bit time64_t into 32-bit integer
        rc = script_func_exec("S.loop", "i", (int)elf::time_ms());
        usleep(50);
    }

    ensure("Run function FAILED.", rc == SCRIPT_RC_OK);
}
}
