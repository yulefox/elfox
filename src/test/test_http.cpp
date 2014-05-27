/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <tut/tut.hpp>

const int TEST_TIMES = 1000;
static elf::time64_t start_time = 0;
static elf::time64_t end_time = 0;
static int req_times = 0;
static int res_times = 0;
static int err_times = 0;

namespace tut {
size_t do_response(void *ptr, size_t size, size_t nmemb, void *userp)
{
    int *times = (int *)userp;

    ++res_times;
    if (!ptr) {
        ++err_times;
        putchar('x');
    } else {
        putchar('.');
    }
    fflush(stdout);
    delete times;
    return size * nmemb;
}

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
    const char *JSON = "{\"csdkId\":\"8000\",\"sdkId\":\"226\",\"uid\":\"123456\",\"token\":\"07d72e5200cb5c0031d988e618d90d5b\",\"ext\":\"扩展\"}";
    set_test_name("HTTP");
<<<<<<< HEAD

    const char *URL = "http://219.133.55.163/api/user/loginoauth.php";
    req_times = 0;
    res_times = 0;
    err_times = 0;
    start_time = elf::time_ms();
    do {
        int *arg = new int(req_times);
        elf::http_json(URL, JSON, do_response, arg);
        usleep(20000);
    } while (++req_times < TEST_TIMES);
    end_time = elf::time_ms();
    do {
        usleep(1000);
    } while (res_times < TEST_TIMES);

    LOG_TEST("%lld - %lld(%lld) req: %d, err: %d.",
            start_time, end_time, end_time - start_time,
            req_times, err_times);

    const char *URL_D = "http://csdk.3333.cn/api/user/loginoauth.php";
    req_times = 0;
    res_times = 0;
    err_times = 0;
    start_time = elf::time_ms();
    do {
        int *arg = new int(req_times);
        elf::http_json(URL_D, JSON, do_response, arg);
        usleep(20000);
    } while (++req_times < TEST_TIMES);
    end_time = elf::time_ms();
    do {
        usleep(1000);
    } while (res_times < TEST_TIMES);
    LOG_TEST("%lld - %lld(%lld) req: %d, err: %d.",
            start_time, end_time, end_time - start_time,
            req_times, err_times);
=======
    void *user = malloc(5);
    while (true) {
        LOG_TEST("%s", "->");
        elf::http_json(URL, JSON, do_response, user);
        usleep(100);
    }
>>>>>>> 3e4aab970f766c98152bcc6197ed5b8ed3dffaec
}
}

