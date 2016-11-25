/*
 * Copyright (C) 2013-2016 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file object.h
 * @author Fox(yulefox@gmail.com)
 * @date 2013-11-01
 * Object/Container base.
 * 3-layer general container for any objects.
 *
 *                   object ID
 *                /     |     \
 *           type A   type B   type C
 *              /       |       \
 *   object ID...  object ID...  object ID...
 *
 * @warning Global object map can ONLY be used in single thread.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#include <elf/config.h>
#include <elf/memory.h>
#include <elf/oid.h>
#include <elf/pb.h>
#include <map>
#include <string>
#include <vector>

namespace elf {
class Object;
class Proto;

typedef std::list<Object *> obj_list;
typedef std::map<oid_t, Object *> obj_map_id;
typedef std::map<int, Object *> obj_map_int;
typedef std::map<std::string, Object *> obj_map_str;
typedef std::map<oid_t, pb_t *> pb_map_id;

class Object {
public:
    ///
    /// Get Object ID.
    /// @return Object ID.
    ///
    inline oid_t ID(void) const { return m_id; }

    ///
    /// Get Object short ID.
    /// @return Short ID.
    ///
    inline int SID(void) const { return m_sid; }

    ///
    /// Get parent ID.
    /// @return Parent ID.
    ///
    inline oid_t PID(void) const { return m_pid; }

    ///
    /// Get Object type.
    /// @return Object type.
    ///
    inline int Type(void) const { return m_type; }

    ///
    /// Get protobuf data.
    /// @return Protobuf data.
    ///
    inline pb_t *PB(void) { return m_pb; }
    inline const pb_t &PB(void) const { return *m_pb; }

    ///
    /// Get Object alias.
    /// @return Alias.
    ///
    inline const std::string &Alias(void) const { return m_alias; }

    ///
    /// Get Object name.
    /// @return Object name.
    ///
    inline const std::string &Name(void) const { return m_name; }

    ///
    /// Set Object name.
    ///
    void SetName(const std::string &name);

    ///
    /// Output statistics info.
    ///
    static void Stat(void);

    ///
    /// Release all Object/Protos.
    ///
    static void Release(void);

    ///
    /// Add protobuf data.
    /// @param pb Protobuf data.
    /// @param pid Parent ID.
    /// @param type Protobuf data type.
    /// @param id Protobuf data ID.
    ///
    static pb_t *AddPB(const pb_t &pb, oid_t id, oid_t pid, int type);

    ///
    /// Clone protobuf data.
    /// @param pb Protobuf data.
    /// @param id Protobuf data ID.
    ///
    static bool ClonePB(pb_t *pb, oid_t id);

    ///
    /// Remove protobuf data.
    /// @param id Protobuf data ID.
    ///
    static void DelPB(oid_t id, oid_t pid, int type);

    ///
    /// Find Object by ID.
    /// @param id Object ID.
    /// @return Pointer to Object if found, or NULL.
    ///
    static Object *FindObject(oid_t id);

    ///
    /// Find protobuf data by ID.
    /// @param id Protobuf data ID.
    /// @return Pointer to protobuf data if found, or NULL.
    ///
    static pb_t *FindPB(oid_t id);

    ///
    /// Find children by container object ID and type in `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type.
    /// @return Pointer to container if found, or NULL.
    ///
    static elf::id_set *GetChildren(elf::oid_t cid, int type);

    ///
    /// Remove children from `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type, clear all if type is 0.
    ///
    static void DelChildren(elf::oid_t cid, int type = 0);

    ///
    /// Find the last/only child object ID by container object ID and type in `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type.
    /// @return Last/Only element object ID.
    ///
    static elf::oid_t GetLastChild(elf::oid_t cid, int type);

    ///
    /// Set the only child object ID into `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type.
    /// @param[in] oid Object ID.
    ///
    static void SetChild(elf::oid_t cid, int type, elf::oid_t oid);

    ///
    /// Insert child object ID into `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type.
    /// @param[in] oid Object ID.
    ///
    static void AddChild(elf::oid_t cid, int type, elf::oid_t oid);

    ///
    /// Remove child object ID from `s_containers`.
    /// @param[in] cid Container ID.
    /// @param[in] type Container type.
    /// @param[in] oid Object ID.
    ///
    static void DelChild(elf::oid_t cid, int type, elf::oid_t oid);

    ///
    /// Get size of object map.
    /// @return Size of object map.
    ///
    static int Size(void) { return s_objs.size(); }

    ///
    /// Get size of protobuf map.
    /// @return Size of object map.
    ///
    static int SizeProto(void) { return s_pbs.size(); }

    virtual ~Object(void);

protected:
    struct Proto {
        pb_t *pb;
        int type;
        int ref;
    };

    ///
    /// Find Proto by ID.
    /// @param[in] id Proto ID.
    /// @return Pointer to Proto object if found, or NULL.
    ///
    static Proto *FindProto(oid_t id);

    ///
    /// Index Proto.
    /// @param[in] id Proto ID.
    /// @param[in] pid Parent ID.
    /// @param[in] type Proto type.
    /// @param[in] id Proto ID.
    /// @return Pointer to Proto object if found, or NULL.
    ///
    static void IndexProto(elf::oid_t id, elf::oid_t pid, int type);

    ///
    /// Unindex Proto.
    /// @param[in] id Proto ID.
    /// @param[in] pid Parent ID.
    /// @param[in] type Proto type.
    /// @param[in] id Proto ID.
    /// @return Pointer to Proto object if found, or NULL.
    ///
    static void UnindexProto(elf::oid_t id, elf::oid_t pid, int type);

    typedef std::map<oid_t, Proto *> proto_map;

    Object();
    Object(oid_t id);

    ///
    /// On initialization, insert Object into global map.
    ///
    virtual void OnInit(void);

    /// Object ID
    oid_t m_id;

    /// short ID
    int m_sid;

    /// parent ID
    oid_t m_pid;

    /// Object ID
    int m_type;

    /// alias
    std::string m_alias;

    /// Object name
    std::string m_name;

    /// protobuf data
    pb_t *m_pb;

    /// global Object map(key: m_id)
    static obj_map_id s_objs;

    /// global Proto map
    static proto_map s_pbs;

    /// global container map
    static elf::id_lismap s_containers;
};
} // namespace elf

