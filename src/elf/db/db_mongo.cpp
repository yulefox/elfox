/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/db/db.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <elf/json.h>
#include <elf/base64.h>
#include <deque>
#include <map>
#include <string>

using namespace google::protobuf;

namespace elf {
enum mongo_query_type {
    QUERY_NO_RES,       // no response
    QUERY_RAW,          // response with raw result
    QUERY_FIELD,        // response with PB field result
    QUERY_PB,           // response with PB result
};

struct mongo_query_t {
    mongo_query_type type;      // type
    std::string collection;     // collection
    std::string selector;       // containing the query to match the document for operations
    bson_t *doc;                // doc
    std::vector<bson_t*> data;  // query result
    oid_t oid;                  // associated object id
    pb_t *pb;                   // store query data
    std::string field;          // pb field
    db_callback proc;           // callback function
    time64_t stamp;             // request time stamp
};

typedef struct mongo_thread_s {
    int idx;
    string dbname;
    thread_t tid;
    mongoc_client_pool_t *pool;
    xqueue<mongo_query_t *> req;
} mongo_thread_t;

typedef struct thread_list_s {
    int idx;
    int num;
    mongo_thread_t *threads;
}thread_list_t;

typedef std::map<int, thread_list_t*> thread_list_map;
static thread_list_map s_threads;

static const int THREAD_NUM_DEFAULT = 5;
static xqueue<mongo_query_t *> s_queue_res;

static void *handle(void *args);
static void query(mongo_thread_t *th);
static void destroy(mongo_query_t *q);
static void response(mongo_query_t *q);

//
static mongoc_client_pool_t *s_pool;

volatile int _mongo_running;

static void *handle(void *args)
{
    mongo_thread_t *self = (mongo_thread_t*)args;

    if (self == NULL) {
        LOG_ERROR("db", "Create thread failed: %s", "mongodb_thread_t is NULL");
        return NULL;
    }

    while (_mongo_running) {
        query(self);
    }
    return NULL;
}

static void query(mongo_thread_t *th)
{
    mongoc_client_pool_t *pool = th->pool;
    mongoc_client_t *client = NULL;
    mongoc_collection_t *collection = NULL;
    bson_t *selector = NULL;
    bson_t *opts = NULL;
    bson_t *doc = NULL;
    mongo_query_t *q = NULL;
    bson_error_t error;

    th->req.pop(q);
    assert(pool &&  q);

    client = mongoc_client_pool_pop(pool);
    collection = mongoc_client_get_collection(client, th->dbname.c_str(), q->collection.c_str());

    selector = bson_new_from_json((const uint8_t*)q->selector.data(), q->selector.size(), &error);
    if (selector == NULL) {
        LOG_ERROR("db", "invalid selecotr:`%s`, %s.", q->selector.c_str(), error.message);
        destroy(q);
        goto CLEANUP;
    }

    if (q->type == QUERY_NO_RES) {
        bson_t *opts = BCON_NEW ("upsert", BCON_BOOL(true));
        if (q->doc == NULL || !mongoc_collection_replace_one(collection, selector, q->doc, opts, NULL, &error)) {
            LOG_ERROR("db", "upsert failed:`%s`, %s.", q->selector.c_str(), error.message);
            destroy(q);
            goto CLEANUP;
        }
    } else {
        opts = BCON_NEW ("projection", "{", "_id", BCON_BOOL (false), "}");
        mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, selector, opts, NULL);
        const bson_t *res;
        if (q->type == QUERY_FIELD) {
            const Reflection *ref = q->pb->GetReflection();
            const Descriptor *des = q->pb->GetDescriptor();
            const FieldDescriptor *ctn = des->FindFieldByName(q->field);
            assert(ctn);
            while (mongoc_cursor_next(cursor, &res)) {
                char *str = bson_as_json(res, NULL);
                pb_t *item = ref->AddMessage(q->pb, ctn);
                if (json2pb(str, item, true) != 0) {
                    LOG_ERROR("db", "json2pb failed: %s", str);
                }
                bson_free(str);
            }
        } else if (q->type == QUERY_PB) {
            if (mongoc_cursor_next(cursor, &res)) {
                char *str = bson_as_json(res, NULL);
                if (json2pb(str, q->pb, true) != 0) {
                    LOG_ERROR("db", "json2pb failed: %s", str);
                }
                bson_free(str);
            }
        } else { // raw
            while (mongoc_cursor_next(cursor, &res)) {
                q->data.push_back(bson_copy(res));
            }
        }

        //
        if (mongoc_cursor_error(cursor, &error)) {
            LOG_ERROR("db", "find failed:`%s`, %s.", q->selector.c_str(), error.message);
        } else {
            // push result
            s_queue_res.push(q);
        }
        mongoc_cursor_destroy(cursor);
    }

CLEANUP:
    // cleanup
    bson_destroy(selector);
    bson_destroy(opts);
    bson_destroy(doc);
    mongoc_client_pool_push (pool, client);
}

static void destroy(mongo_query_t *q)
{
    assert(q);

    for (size_t i = 0;i < q->data.size(); i++) {
        bson_t *item = (bson_t*)q->data[i];
        if (item != NULL) {
            bson_destroy(item);
        }
    }
    q->data.clear();
    if (q->doc != NULL) {
        bson_destroy(q->doc);
    }
    E_DELETE(q->pb);
    E_DELETE(q);
}

int mongodb_init(void)
{
    MODULE_IMPORT_SWITCH;
    _mongo_running = 1;
    mongoc_init ();
    s_pool = NULL;
    return ELF_RC_DB_OK;
}

int mongodb_fini(void)
{
    MODULE_IMPORT_SWITCH;

    _mongo_running = 0;
    thread_list_map::iterator itr = s_threads.begin();
    for (;itr != s_threads.end(); ++itr) {
        thread_list_t *th_list = itr->second;
        for (int i = 0;i < th_list->num; ++i) {
            mongo_thread_t *th = th_list->threads + i;
            thread_fini(th->tid);
        }
        E_DELETE th_list->threads;
        E_DELETE th_list;
    }
    s_threads.clear();

    //
    if (s_pool != NULL) mongoc_client_pool_destroy (s_pool);
    mongoc_cleanup();
    return ELF_RC_DB_OK;
}

int mongodb_connect(int idx, const std::string &uri_str, const std::string &appname, const std::string &dbname, int num)
{
    if (num <= 0) {
        num = THREAD_NUM_DEFAULT;
    }
    thread_list_t *th_list = NULL;

    thread_list_map::iterator itr = s_threads.find(idx);
    if (itr != s_threads.end()) {
        th_list = itr->second;
    } else {
        th_list = E_NEW thread_list_t;
        th_list->idx = idx;
        th_list->num = num;
        th_list->threads = E_NEW mongo_thread_t[num];
        for (int i = 0;i < num; i++) {
            mongo_thread_t *th = th_list->threads + i;
            th->pool = NULL;
        }
        s_threads[idx] = th_list;
    }

    bson_error_t error;
    mongoc_uri_t *uri = mongoc_uri_new_with_error(uri_str.c_str(), &error);
    if (uri == NULL) {
        LOG_ERROR("db", "failed to parse mongodb URI: %s, %s", uri_str.c_str(), error.message);
        return ELF_RC_DB_INIT_FAILED;
    }
   
    // create pool
    s_pool = mongoc_client_pool_new (uri);
    mongoc_client_pool_set_error_api (s_pool, 2);

    for (int i = 0; i < num; ++i) {
        mongo_thread_t *th = th_list->threads + i;
        th->dbname = dbname;
        th->pool = s_pool;
        th->tid = thread_init(handle, (void *)th);
    }

    mongoc_uri_destroy (uri);

    return ELF_RC_DB_OK;
}

int mongodb_proc(void)
{
    if (s_threads.empty()) return -1;

    std::deque<mongo_query_t *> list;
    std::deque<mongo_query_t *>::iterator itr;

    s_queue_res.swap(list);
    for (itr = list.begin(); itr != list.end(); ++itr) {
        mongo_query_t *q = *itr;

        // object has not been destroyed
        response(q);
    }
    return 0;
}

void mongodb_req(int idx, const char *collection, const char *selector, const bson_t *doc, bool parallel, db_callback proc,
        oid_t oid, pb_t *out, const std::string &field)
{
    thread_list_map::iterator itr = s_threads.find(idx);
    if (itr == s_threads.end()) {
        return;
    }
    thread_list_t *th_list = itr->second;

    int tidx = 0;

    mongo_query_t *q = E_NEW mongo_query_t;
    if (proc == NULL && out == NULL) {
        q->type = QUERY_NO_RES;
    } else if (out == NULL) {
        q->type = QUERY_RAW;
    } else if (field == "") {
        q->type = QUERY_PB;
    } else {
        q->type = QUERY_FIELD;
    }

    q->collection = collection;
    q->selector = selector;
    q->doc = NULL;
    if (doc != NULL) {
        q->doc = bson_copy(doc);
    }
    q->stamp = time_ms();
    q->oid = oid;
    q->pb = out;
    q->field = field;
    q->proc = proc;
    q->data.clear();
    if (parallel && oid != 0) {
        tidx = oid % (th_list->num - 1) + 1;
    }
    mongo_thread_t *th = th_list->threads + tidx;
    th->req.push(q);
}

void response(mongo_query_t *q)
{
    assert(q);
    switch (q->type) {
        case QUERY_RAW:
            if (q->proc) {
                q->proc(q->oid, &(q->data));
            }
            break;
        case QUERY_PB:
            if (q->proc) {
                q->proc(q->oid, q->pb);
            }
            break;
        case QUERY_FIELD:
            if (q->proc) {
                q->proc(q->oid, q->pb);
            }
            break;
        default:
            assert(0);
            break;
    }
    destroy(q);
}

size_t mongodb_pending_size(int idx)
{
    size_t sum = 0;
    thread_list_map::iterator itr = s_threads.find(idx);
    if (itr == s_threads.end()) {
        return 0;
    }

    thread_list_t *th_list = itr->second;
    for (int i = 0;i < th_list->num; i++) {
        mongo_thread_t *th = th_list->threads + i;
        sum += th->req.size();
    }
    return sum;
}
} // namespace elf

