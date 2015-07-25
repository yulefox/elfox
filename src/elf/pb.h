/*
 * Copyright (C) 2013-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file elf/pb.h
 * @author Fox(yulefox@gmail.com)
 * @date 2013-12-30
 * @brief Protobuf.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_PB_H
#define ELF_PB_H

#include <elf/config.h>
#include <elf/oid.h>
#include <google/protobuf/message.h>
#include <map>
#include <string>

namespace elf {
typedef ::google::protobuf::Message pb_t;
typedef std::set<pb_t *> pb_set;
typedef std::list<pb_t *> pb_list;
typedef std::map<oid_t, pb_t *> pb_map_id;
typedef std::map<int, pb_t *> pb_map_int;
typedef std::multimap<int, pb_t *> pb_mmap_int;
typedef std::pair <pb_mmap_int::iterator, pb_mmap_int::iterator> pb_mmap_iint;
typedef std::map<std::string, pb_t *> pb_map_str;

typedef pb_t *(*pb_new)(void);

void message_unregist_all(void);

///
/// Register protobuf map.
/// @param[in] name Protobuf type.
/// @param[in] init Protobuf intial function.
///
void pb_regist(const std::string &name, pb_new init);

///
/// Create protobuf object with name.
/// @param[in] name Protobuf type.
/// @return Created protobuf object if registed, or NULL.
///
pb_t *pb_create(const std::string &name);

///
/// Destroy protobuf object.
/// @param[in] pb Protobuf object.
///
void pb_destroy(pb_t *pb);

///
/// Get int32 field of protobuf by number.
/// @param[in] pb Protobuf object.
/// @param[in] num Number.
/// @return Protobuf.
///
int pb_get_int(const pb_t &pb, int num);

///
/// Get field of protobuf(only for type of `Message`).
/// @param[in] pb Protobuf object.
/// @param[in] key Field name.
/// @return Protobuf.
///
pb_t *pb_get_field(pb_t *pb, const std::string &key);

///
/// Set field of protobuf.
/// @param[out] pb Protobuf object.
/// @param[in] key Field name.
/// @param[in] val Field value.
///
void pb_set_field(pb_t *pb, const std::string &key,
        const char *val);

///
/// Set field of protobuf.
/// @param[out] pb Protobuf object.
/// @param[in] fd Field descriptor.
/// @param[in] val Field value.
///
void pb_set_field(pb_t *pb, const ::google::protobuf::FieldDescriptor *fd,
        const char *val);

///
/// Set field of protobuf.
/// @param[out] pb Protobuf object.
/// @param[in] fd Field descriptor.
/// @param[in] val Field value.
/// @param[in] len Field length.
///
void pb_set_field(pb_t *pb, const ::google::protobuf::FieldDescriptor *fd,
        const char *val, int len);
} // namespace elf

#endif /* !ELF_PB_H */

