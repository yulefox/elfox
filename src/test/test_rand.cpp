/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/rand.h>
#include <tut/tut.hpp>

namespace tut {
struct rand {
    rand() {
    }

    ~rand() {
    }
};

typedef test_group<rand> factory;
typedef factory::object object;

static tut::factory tf("rand");

template<>
template<>
void object::test<1>() {
    set_test_name("frand");
    float r = elf::frand(5.0f, 6.0f);
    ensure(r >= 5.0f && r <= 6.0f);

    r = elf::frand(5.0f, 5.0f);
    ensure(r == 5.0f);
}

template<>
template<>
void object::test<2>() {
    set_test_name("rand");
    int r = elf::rand(5, 6);
    ensure(r >= 5 && r <= 6);

    r = elf::rand(5, 5);
    ensure(r == 5);
}

template<>
template<>
void object::test<3>() {
    set_test_name("rand n");
    /*
    elf::roll_set res;

    //elf::rand(11, 20, res, 0);
    ensure(res.size() == 0);
    res.clear();

    //elf::rand(11, 20, res, 10);
    ensure(res.size() == 10);
    res.clear();

    //elf::rand(11, 20, res, 5);
    ensure(res.size() == 5);
    res.clear();
    */
}

template<>
template<>
void object::test<4>() {
    set_test_name("perfect shuffle");

    int n = 8;
    int res[n];
    elf::shuffle_cards(1, n, res);

    printf("\n");
    for (int i = 0; i < n; ++i) {
        printf("%3d:%3d\n", i, res[i]);
    }
}
}
