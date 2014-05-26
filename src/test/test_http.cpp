/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <tut/tut.hpp>

namespace tut {
size_t do_response(void *ptr, size_t size, size_t nmemb, void *userp)
{
    LOG_TEST("%s", ptr);
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
    const char *URL = "http://csdk.3333.cn/api/user/loginoauth.php";
    const char *JSON = "{\"csdkId\":\"8000\",\"sdkId\":\"226\",\"uid\":\"123456\",\"token\":\"07d72e5200cb5c0031d988e618d90d5b\",\"ext\":\"扩展\"}";
    set_test_name("HTTP");
    void *user = malloc(5);
    while (true) {
        LOG_TEST("%s", "->");
        elf::http_json(URL, JSON, do_response, user);
        usleep(100);
    }
}
}


