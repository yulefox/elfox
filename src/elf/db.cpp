/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/db.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <elf/pc.h>
#include <elf/thread.h>
#include <elf/time.h>
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
    elf::time64_t stamp; // request time stamp
};

static MYSQL *s_mysql = NULL;
static thread_t s_tid_req = 0;
static xqueue<query_t *> s_queue_req;
static xqueue<query_t *> s_queue_res;

static void *handle(void *args);
static void query(query_t *q);
static void destroy(query_t *q);
static void response(query_t *q);
static void retrieve_pb(query_t *q);
static void retrieve_field(query_t *q);

static void *handle(void *args)
{
    while (true) {
        query_t *q;

        s_queue_req.pop(q);
        query(q);
    }
    return NULL;
}

static void query(query_t *q)
{
    try {
        assert(q);

        elf::time64_t ct = time_ms();
        elf::time64_t delta = time_diff(ct, q->stamp);
        static elf::time64_t leap = 5000; // 5s
        static elf::time64_t times = 1;

        if (delta > leap * times) {
            LOG_WARN("db", "DB Server is busying for %d.%03ds.",
                    delta / 1000,
                    delta % 1000,
                    q->cmd.c_str());
            ++times;
        } else if (delta < leap) {
            times = 1;
        }

        // query
        int status = mysql_query(s_mysql, q->cmd.c_str());

        if (status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    q->cmd.c_str(), mysql_error(s_mysql));
            destroy(q);
            return;
        }
        if (q->type == QUERY_NO_RES) {
            destroy(q);
            return;
        }

        do {
            q->data = mysql_store_result(s_mysql);
            s_queue_res.push(q);
        } while (!mysql_next_result(s_mysql));
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                q->cmd.c_str(), mysql_error(s_mysql));
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

        assert(ofd);
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

            assert(ofd);
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
    thread_fini(s_tid_req);
    return ELF_RC_DB_OK;
}

int db_connect(const std::string &host, const std::string &user,
        const std::string &passwd, const std::string &db, unsigned int port)
{
    try {
        char value = 1;

        if (s_mysql && 0 == mysql_ping(s_mysql)) {
            return ELF_RC_DB_OK;
        }
        s_mysql = mysql_init(NULL);
        mysql_options(s_mysql, MYSQL_OPT_RECONNECT, (char *)&value);
        mysql_options(s_mysql, MYSQL_SET_CHARSET_NAME, "utf8");
        s_mysql = mysql_real_connect(s_mysql, host.c_str(), user.c_str(),
                passwd.c_str(), db.c_str(), port,
                NULL, CLIENT_MULTI_STATEMENTS);
        if (!s_mysql) {
            LOG_ERROR("db", "Connect DB failed: %s.",
                    mysql_error(s_mysql));
            return ELF_RC_DB_INIT_FAILED;
        }
    } catch(...) {
        LOG_ERROR("db", "Connect DB failed: %s.",
                mysql_error(s_mysql));
        return ELF_RC_DB_INIT_FAILED;
    }
    s_tid_req = thread_init(handle, NULL);
    return ELF_RC_DB_OK;
}

int db_proc(void)
{
    if (s_mysql == NULL) return -1;

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

void db_req(const char *cmd, db_callback proc,
        oid_t oid, pb_t *out, const std::string &field)
{
    query_t *q = E_NEW query_t;

    if (proc == NULL) {
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
    s_queue_req.push(q);
}

void response(query_t *q)
{
    assert(q);
    switch (q->type) {
        case QUERY_RAW:
            q->proc(q->oid, q->data);
            break;
        case QUERY_PB:
            retrieve_pb(q);
            q->proc(q->oid, q->pb);
            break;
        case QUERY_FIELD:
            retrieve_field(q);
            q->proc(q->oid, q->pb);
            break;
        default:
            assert(0);
            break;
    }
    destroy(q);
}

db_rc db_query(const char *cmd)
{
    assert(s_mysql);
    try {
        // query
        int status = mysql_query(s_mysql, cmd);

        if (status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    cmd, mysql_error(s_mysql));
            return ELF_RC_DB_EXECUTE_FAILED;
        }

        do {
            MYSQL_RES *res = mysql_store_result(s_mysql);

            if (res) {
                mysql_free_result(res);
            }
        } while (!mysql_next_result(s_mysql));
        return ELF_RC_DB_OK;
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                cmd, mysql_error(s_mysql));
        return ELF_RC_DB_EXECUTE_FAILED;
    }
    return ELF_RC_DB_OK;
}

db_rc db_query(const char *cmd, pb_t *out)
{
    assert(s_mysql && out);
    LOG_TRACE("db", "Query DB: %s.", cmd);
    try {
        // query
        int status = mysql_query(s_mysql, cmd);

        if (status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    cmd, mysql_error(s_mysql));
            return ELF_RC_DB_EXECUTE_FAILED;
        }

        MYSQL_RES *res = mysql_store_result(s_mysql);

        if (res) {
            const int row_num = mysql_num_rows(res);
            const Descriptor *des = out->GetDescriptor();
            const int field_num = mysql_num_fields(res);
            const MYSQL_ROW row = mysql_fetch_row(res);

            for (int c = 0; row_num > 0 && c < field_num; ++c) {
                const MYSQL_FIELD *ifd =
                    mysql_fetch_field_direct(res, c);
                const FieldDescriptor *ofd =
                    des->FindFieldByName(ifd->name);

                assert(ofd);
                if (row[c] == NULL || (strlen(row[c]) == 0)) {
                    pb_set_field(out, ofd, "");
                } else {
                    pb_set_field(out, ofd, row[c]);
                }
            }
            mysql_free_result(res);
            return ELF_RC_DB_OK;
        } else if (mysql_field_count(s_mysql) == 0) {
            return ELF_RC_DB_OK;
        } else {
            LOG_ERROR("db", "`%s` failed: %s.",
                    cmd, mysql_error(s_mysql));
            return ELF_RC_DB_EXECUTE_FAILED;
        }
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                cmd, mysql_error(s_mysql));
        return ELF_RC_DB_EXECUTE_FAILED;
    }
    return ELF_RC_DB_OK;
}

db_rc db_query(const char *cmd, pb_t *out, const std::string &field)
{
    assert(s_mysql && out);
    LOG_TRACE("db", "Query DB: %s.", cmd);
    try {
        // query
        int status = mysql_query(s_mysql, cmd);

        if (status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    cmd, mysql_error(s_mysql));
            return ELF_RC_DB_EXECUTE_FAILED;
        }

        MYSQL_RES *res = mysql_store_result(s_mysql);

        if (res) {
            const int row_num = mysql_num_rows(res);
            const Reflection *ref = out->GetReflection();
            const Descriptor *des = out->GetDescriptor();
            const FieldDescriptor *ctn = des->FindFieldByName(field);
            const int field_num = mysql_num_fields(res);

            assert(ctn);
            for (int r = 0; r < row_num; ++r) {
                pb_t *item = ref->AddMessage(out, ctn);
                const MYSQL_ROW row = mysql_fetch_row(res);

                des = item->GetDescriptor();
                for (int c = 0; c < field_num; ++c) {
                    const MYSQL_FIELD *ifd =
                        mysql_fetch_field_direct(res, c);
                    const FieldDescriptor *ofd =
                        des->FindFieldByName(ifd->name);

                    assert(ofd);
                    if (row[c] == NULL || (strlen(row[c]) == 0)) {
                        pb_set_field(item, ofd, "");
                    } else {
                        pb_set_field(item, ofd, row[c]);
                    }
                }
            }
            mysql_free_result(res);
            return ELF_RC_DB_OK;
        } else if (mysql_field_count(s_mysql) == 0) {
            return ELF_RC_DB_OK;
        } else {
            LOG_ERROR("db", "`%s` failed: %s.",
                    cmd, mysql_error(s_mysql));
            return ELF_RC_DB_EXECUTE_FAILED;
        }
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                cmd, mysql_error(s_mysql));
        return ELF_RC_DB_EXECUTE_FAILED;
    }
    return ELF_RC_DB_OK;
}
} // namespace elf

