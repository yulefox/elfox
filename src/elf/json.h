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
/// Convert json string to new protobuf object.
/// @param[in] pb Protobuf object.
/// @param[in] json_type json type.
/// @param[in] data json string.
///
void json_pb(pb_t *pb, const char *json_type, const char *data);
} // namespace elf

#endif /* !ELF_JSON_H */

