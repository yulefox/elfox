/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/net/http.h>
#include <elf/thread.h>
#include <curl/curl.h>

namespace elf {
struct http_req_t {
    const char *json;
    const char *url;
    http_response cb;
    void *args;
};

static void *http_post(void *args)
{
    http_req_t *post = (http_req_t *)args;
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl != NULL) {
        struct curl_slist *slist = NULL;

        slist = curl_slist_append(slist, "Expect:");

        curl_easy_setopt(curl, CURLOPT_URL, post->url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post->json);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post->json));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, post->cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, post->args);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            LOG_ERROR("http", "curl_easy_perform() failed(%d): %s %s, %s.",
                    res, curl_easy_strerror(res), post->url, post->json);
            if (post->cb) {
                post->cb(0, 0, 0, post->args);
            }
        }
        curl_slist_free_all(slist);
        curl_easy_cleanup(curl);
    } else {
        LOG_ERROR("http", "%s", "curl_easy_init() failed.");
    }
    E_DELETE post;
    return NULL;
}

int http_init(void)
{
    MODULE_IMPORT_SWITCH;

    /* In windows, this will init the winsock stuff */
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    /* Check for errors */
    if (res != CURLE_OK) {
        LOG_ERROR("http", "curl_global_init() failed: %s.",
                curl_easy_strerror(res));
        return -1;
    }
    return 0;
}

int http_fini(void)
{
    MODULE_IMPORT_SWITCH;

    curl_global_cleanup();
    return 0;
}

int http_json(const char *url, const char *json,
        http_response func, void *args)
{
    http_req_t *post = E_NEW http_req_t;

    post->url = url;
    post->json = json;
    post->cb = func;
    post->args = args;

    elf::thread_init(http_post, post);
    return 0;
}
} // namespace elf

