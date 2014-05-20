/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/thread.h>
#include <tut/tut.hpp>

void *func(void *args)
{
    LOG_INFO("test", "Thread %p starts.", 0);
    LOG_INFO("test", "Thread %p exits.", 0);
    return NULL;
}

namespace tut {
    struct thread {
        thread() {
        }

        ~thread() {
        }
    };

    typedef test_group<thread> factory;
    typedef factory::object object;

    static tut::factory tf("thread");

    template<>
    template<>
    void object::test<1>() {
        set_test_name("Test thread");

        elf::thread_t tid = elf::thread_init(func, NULL);
        usleep(1000);
        elf::thread_fini(tid);
    }
}
