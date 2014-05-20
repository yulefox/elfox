/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/db.h>
#include <stdio.h>
#include <tut/tut.hpp>
/*
#include "cfg/cls.pb.h"
#include "cfg/setup.pb.h"
*/

namespace tut {
struct cp {
    cp() {
    }

    ~cp() {
    }
};

typedef test_group<cp> factory;
typedef factory::object object;

static tut::factory tf("cp");

template<>
template<>
void object::test<1>() {
    /*
    config::Class cfg;

    putchar('\n');
    set_test_name("Load .csv");
    elf::config_load("CONFIG/class.csv", &cfg);
    ensure("'class.csv' gets FAILED.", cfg.item(2).cls() == 10003);
    */
}

template<>
template<>
void object::test<2>() {
    /*
    config::Class cfg;

    putchar('\n');
    elf::db_connect("localhost", "agame", "Root@601", "agame", 3306);
    set_test_name("Load DB table");
    elf::config_load("cfg_role_class", &cfg);
    ensure("'cfg_role_class' gets FAILED.", cfg.item(2).cls() == 10003);
    */
}

template<>
template<>
void object::test<3>() {
    /*
    config::Setup cfg;

    putchar('\n');
    set_test_name("Load .conf");
    elf::config_load("CONFIG/setup.conf", &cfg);
    ensure("Key 'a.b.int' gets FAILED.", cfg.login().port() == 6666);
    */
}
}
