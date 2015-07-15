/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/db.h>
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
    query_type type;    // type
    std::string cmd;    // command
    oid_t oid;          // associated object id
    MYSQL_RES *data;    // query result
    pb_t *pb;           // store query data
    std::string field;  // pb field
    db_callback proc;   // callback function
    time64_t stamp;     // request time stamp
};

static const int THREAD_NUM = 5;
static const int THREAD_INDEX[THREAD_NUM] = {0, 1, 2, 3, 4};
static MYSQL *s_mysqls[THREAD_NUM] = {NULL, NULL, NULL, NULL};
static thread_t s_threads[THREAD_NUM];
static xqueue<query_t *> s_queue_req[THREAD_NUM];
static xqueue<query_t *> s_queue_res;

static void *handle(void *args);
static void query(int idx);
static void destroy(query_t *q);
static void response(query_t *q);
static void retrieve_pb(query_t *q);
static void retrieve_field(query_t *q);

static void *handle(void *args)
{
    int idx = *((int *)args);

    while (true) {
        query(idx);
    }
    return NULL;
}

static void query(int idx)
{
    query_t *q;
    MYSQL *m = s_mysqls[idx];

    try {
        s_queue_req[idx].pop(q);
        assert(m && q);

        // query
        int status = mysql_query(m, q->cmd.c_str());

        if (status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    q->cmd.c_str(), mysql_error(m));
            destroy(q);
            return;
        }
        if (q->type == QUERY_NO_RES) {
            destroy(q);
            return;
        }

        q->data = NULL;
        do {
            MYSQL_RES *res = mysql_store_result(m);

            if (res) {
                if (q->data == NULL) {
                    q->data = res;
                    s_queue_res.push(q);
                } else {
                    mysql_free_result(res);
                }
            }
        } while (!mysql_next_result(m));
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                q->cmd.c_str(), mysql_error(m));
    }
}

static void destroy(query_t *q)
{
    assert(q);

    if (q->data) {
        mysql_free_result(q->data);
    }
    E_DELETE(q->pb);
    E_DELETE(q);
}

static void retrieve_pb(query_t *q)
{
    assert(q && q->data && q->pb);

    const Descriptor *des = q->pb->GetDescriptor();
    const int field_num = mysql_num_fields(q->data);
    const MYSQL_ROW row = mysql_fetch_row(q->data);

    for (int c = 0; c < field_num; ++c) {
        const MYSQL_FIELD *ifd = mysql_fetch_field_direct(q->data, c);
        const FieldDescriptor *ofd = des->FindFieldByName(ifd->name);

        if (ofd == NULL) {
            continue;
        }
        if (row[c] == NULL || (strlen(row[c]) == 0)) {
            pb_set_field(q->pb, ofd, "");
        } else {
            pb_set_field(q->pb, ofd, row[c]);
        }
    }
}

static void retrieve_field(query_t *q)
{
    assert(q && q->data && q->pb);

    const Reflection *ref = q->pb->GetReflection();
    const Descriptor *des = q->pb->GetDescriptor();
    const FieldDescriptor *ctn = des->FindFieldByName(q->field);
    const int row_num = mysql_num_rows(q->data);
    const int field_num = mysql_num_fields(q->data);

    assert(ctn);
    for (int r = 0; r < row_num; ++r) {
        pb_t *item = ref->AddMessage(q->pb, ctn);
        const MYSQL_ROW row = mysql_fetch_row(q->data);

        des = item->GetDescriptor();
        for (int c = 0; c < field_num; ++c) {
            const MYSQL_FIELD *ifd = mysql_fetch_field_direct(q->data, c);
            const FieldDescriptor *ofd =
                des->FindFieldByName(ifd->name);

            if (ofd == NULL) {
                continue;
            }
            if (row[c] == NULL || (strlen(row[c]) == 0)) {
                pb_set_field(item, ofd, "");
            } else {
                pb_set_field(item, ofd, row[c]);
            }
        }
    }
}

int db_init(void)
{
    MODULE_IMPORT_SWITCH;
    return ELF_RC_DB_OK;
}

int db_fini(void)
{
    MODULE_IMPORT_SWITCH;

    for (int i = 0; i < THREAD_NUM; ++i) {
        thread_fini(s_threads[i]);
    }
    return ELF_RC_DB_OK;
}

int db_connect(const std::string &host, const std::string &user,
        const std::string &passwd, const std::string &db, unsigned int port)
{
    for (int i = 0; i < THREAD_NUM; ++i) {
        char value = 1;
        MYSQL *m = s_mysqls[i];

        try {
            if (m && 0 == mysql_ping(m)) {
                return ELF_RC_DB_OK;
            }
            m = mysql_init(NULL);
            mysql_options(m, MYSQL_OPT_RECONNECT, (char *)&value);
            mysql_options(m, MYSQL_SET_CHARSET_NAME, "utf8");
            m = mysql_real_connect(m, host.c_str(), user.c_str(),
                    passwd.c_str(), db.c_str(), port,
                    NULL, CLIENT_MULTI_STATEMENTS);
            if (!m) {
                LOG_ERROR("db", "Connect DB failed: %s.",
                        mysql_error(m));
                return ELF_RC_DB_INIT_FAILED;
            }

            thread_t tid = thread_init(handle, (void *)(THREAD_INDEX + i));

            s_threads[i] = tid;
            s_mysqls[i] = m;
        } catch(...) {
            LOG_ERROR("db", "Connect DB failed: %s.",
                    mysql_error(m));
            return ELF_RC_DB_INIT_FAILED;
        }
    }
    return ELF_RC_DB_OK;
}

int db_proc(void)
{
    if (s_mysqls[0] == NULL) return -1;

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

void db_req(const char *cmd, bool sim, db_callback proc,
        oid_t oid, pb_t *out, const std::string &field)
{
    int idx = 0;
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
    q->cmd = cmd;
    q->stamp = time_ms();
    q->oid = oid;
    q->pb = out;
    q->field = field;
    q->proc = proc;
    q->data = NULL;
    if (sim) {
        idx = oid % (THREAD_NUM - 1) + 1;
    }
    s_queue_req[idx].push(q);
}

void response(query_t *q)
{
    assert(q);
    switch (q->type) {
        case QUERY_RAW:
            if (q->proc) {
                q->proc(q->oid, q->data);
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

size_t db_pending_size(void)
{
    size_t sum = 0;
    for (int i = 0; i < THREAD_NUM; ++i) {
        sum += s_queue_req[i].size();
    }
    return sum;
}
} // namespace elf

