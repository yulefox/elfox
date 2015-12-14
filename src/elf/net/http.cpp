/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/net/http.h>
#include <elf/thread.h>
#include <curl/curl.h>
#include <string>

const static int HTTP_POST_TIMEOUT = 5; // 5 seconds;

namespace elf {
struct http_req_t {
    std::string json;
    std::string url;
    http_response cb;
    void *args;
};

struct http_chunk_t {
    char *data;
    size_t size;
};

static size_t write_memory_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct http_chunk_t *chunk = (struct http_chunk_t*)data;

    chunk->data = (char*)E_REALLOC(chunk->data, chunk->size + realsize + 1);
    if (chunk->data) {
        memcpy((void*)(chunk->data + chunk->size), ptr, realsize);
        chunk->size += realsize;
        chunk->data[chunk->size] = 0;
    }
    return realsize;
}

static void *http_post(void *args)
{
    http_req_t *post = (http_req_t *)args;
    CURL *curl = curl_easy_init();
    CURLcode res;

    http_chunk_t chunk;
    chunk.data = NULL;
    chunk.size = 0;

    if (curl != NULL) {
        struct curl_slist *slist = NULL;

        slist = curl_slist_append(slist, "Content-type:application/json;charset=utf-8");

        //LOG_DEBUG("http", "prepare do post: url[%s], json[%s]", post->url.c_str(), post->json.c_str());
        //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, post->url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post->json.c_str());
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post->json.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_POST_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            LOG_ERROR("http", "curl_easy_perform() failed(%d): %s %s, %s.",
                    res, curl_easy_strerror(res), post->url.c_str(), post->json.c_str());
            if (post->cb) {
                post->cb(0, 0, 0, post->args);
            }
        }
        curl_slist_free_all(slist);
        curl_easy_cleanup(curl);
    } else {
        LOG_ERROR("http", "%s", "curl_easy_init() failed.");
    }

    if (chunk.data == NULL || chunk.size == 0) {
        post->cb(0, 0, 0, post->args);
    } else {
        post->cb((void*)chunk.data, 1, chunk.size, post->args);
        E_FREE(chunk.data);
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
    LOG_DEBUG("http", "url: %s", url);
    LOG_DEBUG("http", "json: %s", json);
    http_req_t *post = E_NEW http_req_t;

    post->url = std::string(url);
    post->json = std::string(json);
    post->cb = func;
    post->args = args;

    thread_init(http_post, post);
    return 0;
}

int urlencode(const char *in, ssize_t size, std::string &out)
{
    CURL *curl = curl_easy_init();
    if(curl) {
        char *buf = curl_easy_escape(curl, in, size);
        if(buf) {
            out = std::string(buf);
            curl_free(buf);
            return 0;
        }
    }
    return -1;
}

} // namespace elf

