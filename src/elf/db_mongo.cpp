/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/db_mongo.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <deque>
#include <map>
#include <string>

using namespace google::protobuf;

namespace elf {
enum query_type {
    QUERY_NO_RES,       // no response
    QUERY_RAW,          // response with raw result
    QUERY_FIELD,        // response with PB field result
    QUERY_PB,           // response with PB result
};

struct query_t {
    query_type type;            // type
    std::string collection;     // collection
    std::string selector;       // containing the query to match the document for operations
    std::string doc;            // doc
    std::vector<bson_t*> data;    // query result
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
    xqueue<query_t *> req;
} mongo_thread_t;

typedef struct thread_list_s {
    int idx;
    int num;
    mongo_thread_t *threads;
}thread_list_t;

typedef std::map<int, thread_list_t*> thread_list_map;
static thread_list_map s_threads;

static const int THREAD_NUM_DEFAULT = 5;
static xqueue<query_t *> s_queue_res;

static void *handle(void *args);
static void query(mongo_thread_t *th);
static void destroy(query_t *q);
static void response(query_t *q);
static void retrieve_pb(query_t *q);
static void retrieve_field(query_t *q);

//
static mongoc_client_pool_t *s_pool;

volatile int _running;

static void *handle(void *args)
{
    mongo_thread_t *self = (mongo_thread_t*)args;

    if (self == NULL) {
        LOG_ERROR("db", "Create thread failed: %s", "mysql_thread_t is NULL");
        return NULL;
    }

    while (_running) {
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
    query_t *q = NULL;
    bson_error_t error;

    th->req.pop(q);
    assert(pool &&  q);

    client = mongoc_client_pool_pop(pool);
    collection = mongoc_client_get_collection(client, th->dbname.c_str(), q->collection.c_str());

    selector = bson_new_from_json((const uint8_t*)q->selector.data(), q->selector.size(), &error);
    if (selector == NULL) {
        LOG_ERROR("db", "invalid selecotr:`%s`, %s.", q->selector.c_str(), error.message);
        goto CLEANUP;
    }

    if (!q->doc.empty()) {
        doc = bson_new_from_json((const uint8_t*)q->doc.data(), q->selector.size(), &error);
        if (doc == NULL) {
            LOG_ERROR("db", "invalid selecotr:`%s`, %s.", q->selector.c_str(), error.message);
            goto CLEANUP;
        }
    }

    if (q->type == QUERY_NO_RES) {
        bson_t *opts = BCON_NEW ("upsert", BCON_BOOL(true));
        if (!mongoc_collection_replace_one(collection, selector, doc, opts, NULL, &error)) {
            LOG_ERROR("db", "upsert failed:`%s`, `%s`, %s.", q->selector.c_str(), q->doc.c_str(), error.message);
            goto CLEANUP;
        }
    } else {
        opts = BCON_NEW ("projection", "{", "_id", BCON_BOOL (false), "}");
        mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, selector, opts, NULL);
        const bson_t *item;
        while (mongoc_cursor_next(cursor, &item)) {
            char *str = bson_as_canonical_extended_json (doc, NULL);
            //q->data.push_back(bson_copy(item));
            bson_free(str);
        }

        //
        if (mongoc_cursor_error (cursor, &error)) {
            LOG_ERROR("db", "find failed:`%s`, %s.", q->selector.c_str(), error.message);
        } else {
            // push result
            s_queue_res.push(q);
        }
        mongoc_cursor_destroy(cursor);


    /*
    const Reflection *ref = q->pb->GetReflection();
    const Descriptor *des = q->pb->GetDescriptor();
    const FieldDescriptor *ctn = des->FindFieldByName(q->field);
    const int row_num = mysql_num_rows(q->data);
    const int field_num = mysql_num_fields(q->data);

    assert(ctn);
    for (int r = 0; r < row_num; ++r) {
        pb_t *item = ref->AddMessage(q->pb, ctn);
        const MYSQL_ROW row = mysql_fetch_row(q->data);
        unsigned long *len = mysql_fetch_lengths(q->data);

        des = item->GetDescriptor();
        for (int c = 0; c < field_num; ++c) {
            const MYSQL_FIELD *ifd = mysql_fetch_field_direct(q->data, c);
            const FieldDescriptor *ofd =
                des->FindFieldByName(ifd->name);

            if (ofd == NULL) {
                continue;
            }
            if (row[c] == NULL || len[c] == 0) {
                pb_set_field(item, ofd, "");
            } else {
                pb_set_field(item, ofd, row[c], len[c]);
            }
        }
    }
    */


    }

CLEANUP:
    // cleanup
    destroy(q);
    bson_destroy(selector);
    bson_destroy(opts);
    bson_destroy(doc);
    mongoc_client_pool_push (pool, client);
}

static void destroy(query_t *q)
{
    assert(q);

    //if (q->data) {
    //    mysql_free_result(q->data);
    //}
    E_DELETE(q->pb);
    E_DELETE(q);
}

static void retrieve_pb(query_t *q)
{
    //assert(q && q->data && q->pb);

    /*
    const Descriptor *des = q->pb->GetDescriptor();
    const int field_num = mysql_num_fields(q->data);
    const MYSQL_ROW row = mysql_fetch_row(q->data);
    size_t *len = mysql_fetch_lengths(q->data);

    for (int c = 0; c < field_num; ++c) {
        const MYSQL_FIELD *ifd = mysql_fetch_field_direct(q->data, c);
        const FieldDescriptor *ofd = des->FindFieldByName(ifd->name);

        if (ofd == NULL) {
            continue;
        }
        if (row[c] == NULL || (strlen(row[c]) == 0)) {
            pb_set_field(q->pb, ofd, "");
        } else {
            pb_set_field(q->pb, ofd, row[c], len[c]);
        }
    }
    */
}

static void retrieve_field(query_t *q)
{
    //assert(q && q->data && q->pb);

    /*
    const Reflection *ref = q->pb->GetReflection();
    const Descriptor *des = q->pb->GetDescriptor();
    const FieldDescriptor *ctn = des->FindFieldByName(q->field);
    const int row_num = mysql_num_rows(q->data);
    const int field_num = mysql_num_fields(q->data);

    assert(ctn);
    for (int r = 0; r < row_num; ++r) {
        pb_t *item = ref->AddMessage(q->pb, ctn);
        const MYSQL_ROW row = mysql_fetch_row(q->data);
        unsigned long *len = mysql_fetch_lengths(q->data);

        des = item->GetDescriptor();
        for (int c = 0; c < field_num; ++c) {
            const MYSQL_FIELD *ifd = mysql_fetch_field_direct(q->data, c);
            const FieldDescriptor *ofd =
                des->FindFieldByName(ifd->name);

            if (ofd == NULL) {
                continue;
            }
            if (row[c] == NULL || len[c] == 0) {
                pb_set_field(item, ofd, "");
            } else {
                pb_set_field(item, ofd, row[c], len[c]);
            }
        }
    }
    */
}

int mongodb_init(void)
{
    MODULE_IMPORT_SWITCH;
    _running = 1;
    mongoc_init ();
    s_pool = NULL;
    return ELF_RC_DB_OK;
}

int mongodb_fini(void)
{
    MODULE_IMPORT_SWITCH;

    _running = 0;
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

int mongodb_connect(int idx, const std::string &uri_str, const std::string &appname, const std::string &db, int num)
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
        th->pool = s_pool;
        th->tid = thread_init(handle, (void *)th);
    }

    mongoc_uri_destroy (uri);

    return ELF_RC_DB_OK;
}

int mongodb_proc(void)
{
    if (s_threads.empty()) return -1;

    std::deque<query_t *> list;
    std::deque<query_t *>::iterator itr;

    s_queue_res.swap(list);
    for (itr = list.begin(); itr != list.end(); ++itr) {
        query_t *q = *itr;

        // object has not been destroyed
        response(q);
    }
    return 0;
}

void mongodb_req(int idx, const char *selector, const char *doc, bool parallel, db_callback proc,
        oid_t oid, pb_t *out, const std::string &field)
{
    thread_list_map::iterator itr = s_threads.find(idx);
    if (itr == s_threads.end()) {
        return;
    }
    thread_list_t *th_list = itr->second;

    int tidx = 0;
    query_t *q = E_NEW query_t;

    if (proc == NULL && out == NULL) {
        q->type = QUERY_NO_RES;
    } else if (out == NULL) {
        q->type = QUERY_RAW;
    } else if (field == "") {
        q->type = QUERY_PB;
    } else {
        q->type = QUERY_FIELD;
    }
    q->selector = selector;
    q->doc = doc;
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

void response(query_t *q)
{
    assert(q);
    switch (q->type) {
        case QUERY_RAW:
            if (q->proc) {
                q->proc(q->oid, NULL);
            }
            break;
        case QUERY_PB:
            retrieve_pb(q);
            if (q->proc) {
                q->proc(q->oid, q->pb);
            }
            break;
        case QUERY_FIELD:
            retrieve_field(q);
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

