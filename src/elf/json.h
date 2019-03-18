/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file elf/json.h
 * @author Fox(yulefox@gmail.com)
 * @date 2014-05-20
 * @brief JSON.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_JSON_H
#define ELF_JSON_H

#include <elf/config.h>
#include <elf/pb.h>

namespace elf {
///
/// Bind json protocol.
/// @param[in] path json protocol file name.
/// @return true if done, or false.
///
bool json_bind(const char *path);

///
/// Unbind json protocol.
/// @param[in] path json protocol file name.
/// @return true if done, or false.
///
bool json_unbind(const char *path);

///
/// Convert json string to new protobuf object.
/// @param[in] pb_type Protobuf type.
/// @param[in] json_type json type.
/// @param[in] data json string.
/// @return Created protobuf object if converted done, or NULL.
///
pb_t *json_pb(const char *pb_type, const char *json_type, const char *data);

///
/// Convert json string to existed protobuf object.
/// @param[in] pb Protobuf object.
/// @param[in] json_type json type.
/// @param[in] data json string.
/// @return Converted protobuf object if converted done, or NULL.
///
pb_t *json_pb(pb_t *pb, const char *json_type, const char *data);

///
/// Convert json string to new protobuf object.
/// @param[in] pb_type Protobuf type.
/// @param[in] json json string.
/// @return Created protobuf object if converted done, or NULL.
///
pb_t *json_pb(const std::string &pb_type, const std::string &json);

///
/// Convert json string to existed protobuf object.
/// @param[in] json json string.
/// @param[in out] pb Protobuf object.
/// @return 0 if converted done.
///
int json2pb(const std::string &json, pb_t *pb, bool ignore_unknown_fields = false);

///
/// Convert protobuf object to existed json string.
/// @param[in] pb Protobuf object.
/// @param[in out] json json string.
/// @return 0 if converted done.
///
int pb2json(const pb_t &pb, std::string *json);
} // namespace elf

#endif /* !ELF_JSON_H */

