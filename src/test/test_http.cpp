/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <tut/tut.hpp>

namespace tut {
struct http {
    http() {
    }

    ~http() {
    }
};

typedef test_group<http> factory;
typedef factory::object object;

static tut::factory tf("http");

template<>
template<>
void object::test<1>() {
    set_test_name("HTTP");
    ensure(true);
}
}
