/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file db.h
 * @author Fox (yulefox at gmail.com)
 * @date 2012-12-05
 * @brief Database operations: CRUD.
 *
 * Use MySQL.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_DB_H
#define ELF_DB_H

#include <elf/config.h>
#include <elf/object.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <elf/time.h>
#include <mysql/mysql.h>
#include <mongoc/mongoc.h>
#include <string>
#include <deque>

enum db_rc {
    ELF_RC_DB_OK,
    ELF_RC_DB_INIT_FAILED,
    ELF_RC_DB_SYNTAX_ERROR,
    ELF_RC_DB_COMPILE_FAILED,
    ELF_RC_DB_EXECUTE_FAILED,
    ELF_RC_DB_TABLE_NOT_FOUND,
    ELF_RC_DB_COMMAND_NOT_FOUND,
};

namespace elf {
// DB query session callback function
typedef void (*db_callback)(oid_t, void *);

/**
 * Initialize the DB module.
 * @return ELF_RC_DB_OK(0).
 */
int db_init(void);

/**
 * Release the DB module.
 * @return ELF_RC_DB_OK(0).
 */
int db_fini(void);

/**
 * Process all query session.
 * @return (0).
 */
int db_proc(void);

/**
 * Initialize the DB module.
/// @param[in] idx DB index.
 * @return ELF_RC_DB_OK(0).
 */
int db_connect(int idx, const std::string &host, const std::string &user,
        const std::string &passwd, const std::string &db, unsigned int port,
        int threads);

/**
 * Check connection, reconnect if dropped.
 * @return ELF_RC_DB_OK(0).
 */
int db_ping(void);

///
/// DB request(asynchronous).
/// @param[in] idx DB index.
/// @param[in] cmd SQL command.
/// @param[in] parallel Run parallel.
/// @param[in] proc Callback function.
/// @param[in] oid Object id for checking.
/// @param[out] out Store query data.
/// @param[in] field pb field.
///
void db_req(int idx, const char *cmd, bool parallel = false, db_callback proc = NULL,
        oid_t oid = 0, pb_t *out = NULL,
        const std::string &field = "");

///
/// Get size of pending request queues.
/// @param[in] idx DB index.
/// @return Size of pending request queues.
///
size_t db_pending_size(int idx);






////////////////////////////
int mongodb_init(void);
int mongodb_fini(void);
int mongodb_proc(void);
int mongodb_connect(int idx, const std::string &uri_str, const std::string &appname, const std::string &db, int threads);
int mongodb_ping(void);
void mongodb_req(int idx, const char *collection, const char *selector, const bson_t *doc, bool parallel = false, db_callback proc = NULL,
        oid_t oid = 0, pb_t *out = NULL,
        const std::string &field = "");
size_t mongodb_pending_size(int idx);


} // namespace elf

#endif /* !ELF_DB_H */

