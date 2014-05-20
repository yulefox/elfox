/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <tut/tut.hpp>

namespace tut {
elf::xqueue<int> queue;

void *producer(void *arg)
{
    int i = 0;

    while (true) {
        queue.push(++i);
        LOG_TEST("P: %d", i);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg)
{
    while (true) {
        int i = 0;

        queue.pop(i);
        LOG_TEST("C: %d", i);
    }
    return NULL;
}

struct pc {
    pc() {
    }

    ~pc() {
    }
};

typedef test_group<pc> factory;
typedef factory::object object;

static tut::factory tf("pc");

template<>
template<>
void object::test<1>() {
    set_test_name("PC");

    elf::thread_t cid = elf::thread_init(consumer, NULL);
    elf::thread_t pid = elf::thread_init(producer, NULL);

    elf::thread_fini(cid);
    elf::thread_fini(pid);
}
}
