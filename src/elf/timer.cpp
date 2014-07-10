/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/log.h>
#include <elf/memory.h>
#include <elf/time.h>
#include <elf/timer.h>
#include <algorithm>
#include <list>

namespace elf {
#define WHEEL_SET_BIT0                  8
#define WHEEL_SET_BITS                  (WHEEL_SET_BIT0 * 4)
#define WHEEL_SET_BIT(l)                (WHEEL_SET_BIT0 * (l))
#define WHEEL_SET_SIZE0                 (1 << WHEEL_SET_BIT0)
#define WHEEL_SET_SIZE(l)               WHEEL_SET_SIZE0
#define MAX_WHEEL_SET_SIZE              (WHEEL_SET_SIZE0 * 4)
#define MAX_CURSOR                      (1LL << WHEEL_SET_BITS)
#define MAX_LIFE                        (MAX_CURSOR * s_mgr.interval)
#define FRAME_DIFFER(l, r)              (int)(l - r)

union cursor_t {
    struct wheel_t {
        unsigned char w0;
        unsigned char w1;
        unsigned char w2;
        unsigned char w3;
    };
    wheel_t w;
    int a;
};

struct timer_t {
    oid_t id; // identification
    time64_t life; // life from now
    cursor_t cursor; // expired frame
    bool script;
    union cb_t {
        char script[1024]; // script function name
        callback func; // callback function
    };
    cb_t cb; // callback function
    void *args; // callback arguments
    timer_t *next; // the next node of the tail node is NULL
    timer_t *prev; // the previous node of the head node is the tail
};

struct mgr_t {
    time64_t start_time; // start time
    time64_t end_time; // end time
    time64_t cur_time; // current frame time
    time64_t last_time; // last pause time
    time64_t interval; // frame interval time
    cursor_t cursor; // current cursor (number of escaped frames)
    int round; // current round
    int last_cursor; // last pause cursor
    int timer_total; // total number of timers
    int timer_remain; // remain timers
    int timer_passed; // total number of passed timers
    int timer_cancelled; // total number of cancelled timers
    bool pause; // suspend all timers
    timer_t *timers[MAX_WHEEL_SET_SIZE];
};

typedef std::list<callback> handler_list;

static handler_list s_handlers;

static const time64_t TIMER_FRAME_INTERVAL_DEFAULT = 50; // (ms)
static const time64_t TIMER_FRAME_INTERVAL_MIN = 1; // (frame)
static const time64_t TIMER_FRAME_INTERVAL_MAX = 1000; // (frame)
static mgr_t s_mgr;

/// calculate the number of life frames
#define FRAME_CALC(t) ((time64_t)(t) / s_mgr.interval)

// @{
/// get current bucket cursor at wheel 0
#define CURRENT_WHEEL_CURSOR    s_mgr.cursor.w.w0

/// compare given cursor with current bucket cursor at wheel @param l
#define WHEEL_CMP(cur, l)                                                   \
    ((int)cur.w.w##l - (int)s_mgr.cursor.w.w##l)

/// map cursor at wheel @param l
#define BUCKET_MAP(cur, l)                                                  \
    (cur.w.w##l + WHEEL_SET_SIZE0 * l)

/// calculate the number of frames elapsed from last cursor
#define FRAME_BINGO (s_mgr.last_cursor +                                    \
        FRAME_CALC(s_mgr.cur_time - s_mgr.last_time) - s_mgr.cursor.a)
// @}


static void reset(void);
static void push(timer_t *t, int bucket);
static timer_t *get(const oid_t &tid, int *bucket);
static void destroy(timer_t *t);
static int schedule(timer_t *t);
static void expire(timer_t *t);
static void bingo(void);
static void hash(int bucket);
static bool timer_min(void *args);

int timer_init(void)
{
    MODULE_IMPORT_SWITCH;
    s_mgr.interval = TIMER_FRAME_INTERVAL_DEFAULT;
    s_mgr.start_time = s_mgr.last_time = time_ms();
    s_mgr.pause = true;
    s_mgr.cursor.a = 0;
    s_mgr.round = 0;
    memset(s_mgr.timers, 0, sizeof(s_mgr.timers[0]) * MAX_WHEEL_SET_SIZE);

    time_t ms = elf::time_ms() % 1000;
    time_t cur = elf::time_s();
    elf::time64_t tm_timer;
    struct tm tm_cur;

    localtime_r(&cur, &tm_cur);
    tm_timer = (60 - tm_cur.tm_sec) * 1000llu - ms;

    timer_add(tm_timer, timer_min, NULL);
    return 0;
}

int timer_fini(void)
{
    MODULE_IMPORT_SWITCH;
    s_mgr.end_time = time_ms();
    for (int i = 0; i < MAX_WHEEL_SET_SIZE; ++i) {
        timer_t *head = s_mgr.timers[i];
        timer_t *t = head;

        while (t) {
            timer_t *n = t->next;

            destroy(t);
            if (n == head) break;
            t = n;
        }
    }
    return 0;
}

void timer_run(void)
{
    if (s_mgr.pause || s_mgr.timer_remain <= 0) {
        return;
    }

    // frame may greater than 1 while CPU busying
    s_mgr.cur_time = time_ms();
    int frame = FRAME_BINGO;

    // time anomaly
    if (frame < 0) {
        LOG_WARN("timer", "(%u)%08X: %llu - %llu(%d).",
                s_mgr.round, s_mgr.cursor.a,
                s_mgr.cur_time, s_mgr.last_time,
                frame);
        assert(frame >= 0);
        return;
    }
    // while processors is busy
    if (frame > 1) {
        LOG_WARN("timer", "(%u)%08X: %llu - %llu(%d).",
                s_mgr.round, s_mgr.cursor.a,
                s_mgr.cur_time, s_mgr.last_time,
                frame);
    }
    for (int i = 1; i <= frame; ++i) { // BINGO!
        unsigned char bucket = CURRENT_WHEEL_CURSOR;
        timer_t *head = s_mgr.timers[bucket];
        timer_t *t = head;

        while (t) {
            timer_t *n = t->next;

            expire(t);
            ++s_mgr.timer_passed;
            --s_mgr.timer_remain;
            if (n == head) {
                s_mgr.timers[bucket] = NULL;
                break;
            }
            t = n;
        }
        bingo();
    }
}

void timer_stat(void)
{
    LOG_INFO("timer",
            "ST: %lld RND: %u FRM: %u TMT: %u TMP: %u TMC: %u.",
            s_mgr.start_time, s_mgr.round, s_mgr.cursor.a,
            s_mgr.timer_total, s_mgr.timer_passed, s_mgr.timer_cancelled);
}

int timer_size(void)
{
    return s_mgr.timer_remain;
}

const oid_t &timer_add(time64_t life, const char *func)
{
    if (life < 0 || life >= MAX_LIFE) {
        LOG_WARN("timer",
                "Timer added FAILED: invalid timer life(%lld) [0, %lld).",
                life, MAX_LIFE);
        return OID_NIL;
    }

    timer_t *t = E_NEW timer_t;

    memset(t, 0, sizeof(*t));
    t->id = oid_gen();
    t->life = std::max(life, s_mgr.interval);
    t->cursor.a = (FRAME_CALC(t->life) + s_mgr.cursor.a) % MAX_CURSOR;
    t->script = true;
    strcpy(t->cb.script, func);
    t->args = NULL;
    schedule(t);
    ++s_mgr.timer_total;
    ++s_mgr.timer_remain;
    return t->id;
}

const oid_t &timer_add(time64_t life, callback func, void *args)
{
    if (life < 0 || life >= MAX_LIFE) {
        LOG_WARN("timer",
                "Timer added FAILED: invalid timer life(%lld) [0, %lld).",
                life, MAX_LIFE);
        return OID_NIL;
    }

    timer_t *t = E_NEW timer_t;

    memset(t, 0, sizeof(*t));
    t->id = oid_gen();
    t->life = std::max(life, s_mgr.interval);
    t->cursor.a = (FRAME_CALC(t->life) + s_mgr.cursor.a) % MAX_CURSOR;
    t->script = false;
    t->cb.func = func;
    t->args = args;
    schedule(t);
    ++s_mgr.timer_total;
    ++s_mgr.timer_remain;
    return t->id;
}

void timer_cycle(callback func)
{
    s_handlers.push_back(func);
}

void timer_remove(const oid_t &tid)
{
    int bucket;
    timer_t *t = get(tid, &bucket);

    if (t) {
        timer_t *n = t->next;

        if (n != t) {
            t->prev->next = n;
            n->prev = t->prev;
        } else {
            n = NULL;
        }
        if (t == s_mgr.timers[bucket]) {
            s_mgr.timers[bucket] = n;
        }
        destroy(t);
        ++s_mgr.timer_cancelled;
        --s_mgr.timer_remain;
    }
}

void timer_pause(void)
{
    if (!s_mgr.pause) {
        s_mgr.pause = true;
    }
}

void timer_resume(void)
{
    if (s_mgr.pause) {
        s_mgr.pause = false;
        reset();
    }
}

void timer_pause(const oid_t &tid)
{
}

void timer_resume(const oid_t &tid)
{
}

void timer_interval(time64_t t)
{
    ELF_ASSERT(t >= TIMER_FRAME_INTERVAL_MIN && t <= TIMER_FRAME_INTERVAL_MAX);
    s_mgr.interval = t;
    reset();
}

void timer_bucket(unsigned char no, int l)
{
    switch (l) {
        case 0:
            s_mgr.cursor.w.w0 = no;
            break;
        case 1:
            s_mgr.cursor.w.w1 = no;
            break;
        case 2:
            s_mgr.cursor.w.w2 = no;
            break;
        case 3:
            s_mgr.cursor.w.w3 = no;
            break;
    }
    reset();
}

static void reset(void)
{
    s_mgr.last_time = time_ms();
    s_mgr.last_cursor = s_mgr.cursor.a;
}

static void push(timer_t *t, int bucket)
{
    timer_t *head = s_mgr.timers[bucket];

    if (head) {
        head->prev->next = t;
        t->prev = head->prev;
    } else {
        head = s_mgr.timers[bucket] = t;
    }
    head->prev = t;
    t->next = head;
}

static timer_t *get(const oid_t &tid, int *bucket)
{
    timer_t *head = NULL;
    timer_t *t = NULL;

    for (int i = 0; i < MAX_WHEEL_SET_SIZE; ++i) {
        head = t = s_mgr.timers[i];
        while (t) {
            if (tid == t->id) {
                *bucket = i;
                return t;
            }
            if ((t = t->next) == head) break;
        }
    }
    *bucket = 0;
    return NULL;
}

static void destroy(timer_t *t)
{
    if (t) {
        E_FREE(t->args);
        S_DELETE(t);
    }
}

static int schedule(timer_t *t)
{
    int frame = FRAME_DIFFER(t->cursor.a, s_mgr.cursor.a);
    int bucket = 0;

    if (frame < WHEEL_SET_SIZE0) { // hash into wheel 0
        bucket = BUCKET_MAP(t->cursor, 0);
    } else if (WHEEL_CMP(t->cursor, 3)) { // hash into wheel 3
        bucket = BUCKET_MAP(t->cursor, 3);
    } else if (WHEEL_CMP(t->cursor, 2)) { // hash into wheel 2
        bucket = BUCKET_MAP(t->cursor, 2);
    } else if (WHEEL_CMP(t->cursor, 1)) { // hash into wheel 1
        bucket = BUCKET_MAP(t->cursor, 1);
    }
    push(t, bucket);
    return bucket;
}

static void expire(timer_t *t)
{
    ELF_ASSERT(t);
    if (t->script) {
        // script_func_exec(t->cb.script, 0);
    } else {
        t->cb.func(t->args);
    }
    destroy(t);
}

static void bingo(void)
{
    int bucket = 0;
    cursor_t next_cursor = s_mgr.cursor;

    ++next_cursor.a;
    if (WHEEL_CMP(next_cursor, 3)) {
        bucket = BUCKET_MAP(next_cursor, 3);
    } else if (WHEEL_CMP(next_cursor, 2)) {
        bucket = BUCKET_MAP(next_cursor, 2);
    } else if (WHEEL_CMP(next_cursor, 1)) {
        bucket = BUCKET_MAP(next_cursor, 1);
    }
    if (++s_mgr.cursor.a == 0) {
        ++s_mgr.round;
    }
    if (bucket > 0) { // hash to more precisely wheel
        hash(bucket);
    }
}

static void hash(int bucket)
{
    timer_t *head = s_mgr.timers[bucket];
    timer_t *t = head;

    while (t) {
        timer_t *n = t->next;

        schedule(t);
        if (n == head) {
            s_mgr.timers[bucket] = NULL;
            break;
        }
        t = n;
    }
}
static bool timer_min(void *args)
{
    LOG_TRACE("timer", "%s", "BINGO: MIN.");

    time_t cur = elf::time_s();
    elf::time64_t tm_timer;
    struct tm tm_cur;

    localtime_r(&cur, &tm_cur);
    tm_timer = (60 - tm_cur.tm_sec) * 1000llu;
    timer_add(tm_timer, timer_min, NULL);

    handler_list::const_iterator itr = s_handlers.begin();

    for (; itr != s_handlers.end(); ++itr) {
        (*itr)(&cur);
    }
    return true;
}
} // namespace elf

