/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/oid.h>
#include <elf/time.h>
#include <elf/timer.h>
#include <tut/tut.hpp>

#define TIMER_MAX_LIFE      20000
#define TIMER_NUMBER        10
#define FPS                 200

namespace tut {
struct timer {
    timer() {
    }

    ~timer() {
    }
};

typedef test_group<timer> factory;
typedef factory::object object;

static tut::factory tf("timer");

template<>
template<>
void object::test<1>() {
    set_test_name("Add");
    putchar('\n');
    elf::timer_resume();
    for (int i = 0; i < TIMER_NUMBER; ++i) {
        elf::timer_add(rand() % TIMER_MAX_LIFE, "timer.onTimeout");
    }
    elf::timer_stat();
}

template<>
template<>
void object::test<2>() {
    set_test_name("Run");
    putchar('\n');

    elf::time64_t st = elf::time_ms();

    for (int i = 0; i < TIMER_NUMBER; ++i) {
        elf::timer_add(rand() % TIMER_MAX_LIFE, "timer.onTimeout");
    }

    while (elf::timer_size()) {
        elf::time64_t time = elf::time_ms();

        elf::timer_run();

        // Clamp the framerate so that we do not hog all the CPU.
        const int MIN_FRAME_TIME = 1000 / FPS;
        elf::time64_t last_frame = time;
        int dt = (int)elf::time_diff(last_frame, time);

        last_frame = time;
        if (dt < MIN_FRAME_TIME) {
            int ms = MIN_FRAME_TIME - dt;
            if (ms >= 0)
                usleep(ms);
        }
    }

    elf::time64_t delta = elf::time_diff(elf::time_ms(), st);

    LOG_TEST("Run %d timers: %lld.%03lld s",
            TIMER_NUMBER,
            delta / 1000, delta % 1000);
    elf::timer_stat();
}

template<>
template<>
void object::test<3>() {
    set_test_name("End");
    putchar('\n');
}
}
