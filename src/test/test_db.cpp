/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <tut/tut.hpp>

namespace tut {
struct db {
    db() {
    }

    ~db() {
    }
};

typedef test_group<db> factory;
typedef factory::object object;

static tut::factory tf("db");

template<>
template<>
void object::test<1>() {
    set_test_name("Create DB");
    elf::db_connect(0, "127.0.0.1", "agame", "Root@601", "agame", 3306, 4);
    elf::db_req(0, "select 5 + 3");
    ensure(true);
}
}
