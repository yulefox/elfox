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
struct db_req_t {
    bool read; // need restore query result
    std::string cmd; // sql command
    oid_t sid; // session id
};

struct db_res_t {
    int status; // status
    MYSQL_RES *data; // query result
    oid_t sid; // session id
};

struct session_t {
    oid_t id; // session id
    oid_t oid; // associated object id
    pb_t *out; // store query data
    std::string field; // pb field
    db_callback proc; // callback function
};

typedef std::map<oid_t, session_t *> session_list;

static session_list s_sessions;
static MYSQL *s_mysql = NULL;
static thread_t s_tid_req = 0;
static xqueue<db_req_t *> s_queue_req;
static xqueue<db_res_t *> s_queue_res;

static void *handle(void *args);
static void query(db_req_t *req);
static void retrieve(MYSQL_RES *res, pb_t *out);
static void retrieve(MYSQL_RES *res, pb_t *out, const std::string &field);

static void *handle(void *args)
{
    while (true) {
        db_req_t *req;

        s_queue_req.pop(req);
        query(req);
        E_DELETE(req);
    }
    return NULL;
}

static void query(db_req_t *req)
{
    try {
        assert(req);

        // query
        int status = mysql_query(s_mysql, req->cmd.c_str());
        db_res_t *res = E_NEW db_res_t;

        res->sid = req->sid;
        res->status = status;
        res->data = NULL;

        if (res->status != 0) {
            LOG_ERROR("db", "`%s` failed: %s.",
                    req->cmd.c_str(), mysql_error(s_mysql));
        } else {
            do {
                MYSQL_RES *data = mysql_store_result(s_mysql);

                if (data) {
                    if (req->read) {
                        res->data = data;
                    } else {
                        mysql_free_result(data);
                    }
                }
            } while (!mysql_next_result(s_mysql));
        }
        s_queue_res.push(res);
    } catch(...) {
        LOG_ERROR("db", "`%s` failed: %s.",
                req->cmd.c_str(), mysql_error(s_mysql));
    }
}

static void retrieve(MYSQL_RES *res, pb_t *out)
{
    const Descriptor *des = out->GetDescriptor();
    const int field_num = mysql_num_fields(res);
    const MYSQL_ROW row = mysql_fetch_row(res);

    for (int c = 0; c < field_num; ++c) {
        const MYSQL_FIELD *ifd = mysql_fetch_field_direct(res, c);
        const FieldDescriptor *ofd = des->FindFieldByName(ifd->name);

        assert(ofd);
        if (row[c] == NULL || (strlen(row[c]) == 0)) {
            pb_set_field(out, ofd, "");
        } else {
            pb_set_field(out, ofd, row[c]);
        }
    }
}

static void retrieve(MYSQL_RES *res, pb_t *out, const std::string &field)
{
    const Reflection *ref = out->GetReflection();
    const Descriptor *des = out->GetDescriptor();
    const FieldDescriptor *ctn = des->FindFieldByName(field);
    const int row_num = mysql_num_rows(res);
    const int field_num = mysql_num_fields(res);

    assert(ctn);
    for (int r = 0; r < row_num; ++r) {
        pb_t *item = ref->AddMessage(out, ctn);
        const MYSQL_ROW row = mysql_fetch_row(res);

        des = item->GetDescriptor();
        for (int c = 0; c < field_num; ++c) {
            const MYSQL_FIELD *ifd = mysql_fetch_field_direct(res, c);
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

    std::deque<db_res_t *> list;
    std::deque<db_res_t *>::iterator itr;

    s_queue_res.swap(list);
    for (itr = list.begin(); itr != list.end(); ++itr) {
        db_res_t *res = *itr;
        session_list::iterator itr_s = s_sessions.find(res->sid);

        assert(itr_s != s_sessions.end());

        session_t *s = itr_s->second;

        assert(s);

        // object has not been destroyed
        if (res->status == 0 && res->data != NULL
                && s->out != NULL && s->proc != NULL) {
            db_res(res->data, s->out, s->field);
            s->proc(s->oid, s->out);
        }
        if (res->data != NULL) {
            mysql_free_result(res->data);
        }
        E_DELETE(res);
    }
    return 0;
}

void db_req(const char *cmd, oid_t oid, pb_t *out,
        const std::string &field, db_callback proc)
{
    db_req_t *req = E_NEW db_req_t;
    session_t *s = E_NEW session_t;

    s->id = oid_gen();
    s->oid = oid;
    s->out = out;
    s->field = field;
    s->proc = proc;
    s_sessions[s->id] = s;

    req->cmd = cmd;
    req->sid = s->id;
    req->read = (out != NULL);
    s_queue_req.push(req);
}

void db_res(MYSQL_RES *res, pb_t *out, const std::string &field)
{
    assert(res && out);

    if (field == "") {
        retrieve(res, out);
    } else {
        retrieve(res, out, field);
    }
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

