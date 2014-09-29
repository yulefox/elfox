/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file net/http.h
 * @author Fox(yulefox@gmail.com)
 * @date 2014-05-15
 * @brief HTTP request/response.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_NET_HTTP_H
#define ELF_NET_HTTP_H

#include <elf/config.h>

namespace elf {
typedef size_t (*http_response)(void *ptr, size_t size, size_t nmemb, void *args);

///
/// Initialize the HTTP module.
/// @return (0).
///
int http_init(void);

///
/// Release the HTTP module.
/// @return (0).
///
int http_fini(void);

///
/// HTTP request in json.
/// @param url URL.
/// @param json json.
/// @param func HTTP response callback handle.
/// @param args Callback arguments.
/// @return (0).
///
int http_json(const char *url, const char *json,
        http_response func, void *args);
} // namespace elf

#endif /* !ELF_NET_HTTP_H */

