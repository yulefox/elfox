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

typedef std::list<Object *> obj_list;
typedef std::map<oid_t, Object *> obj_map_id;
typedef std::map<int, Object *> obj_map_int;
typedef std::map<std::string, Object *> obj_map_str;
typedef std::map<oid_t, pb_t *> pb_map_id;

struct Proto {
    pb_t *pb;
    oid_t id;
    oid_t uid;
    oid_t pid;
    int ptype;
    int type;
    int idx;
    int ref;
};

class Object {
public:
    ///
    /// Get Object ID.
    /// @return Object ID.
    ///
    inline oid_t ID(void) const { return m_id; }

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
    /// Get Object index.
    /// @return Object index.
    ///
    inline int Index(void) const { return m_idx; }

    ///
    /// Get Object short ID.
    /// @return Short ID.
    ///
    inline int SID(void) const { return m_sid; }

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
    /// Get protobuf data.
    /// @return Protobuf data.
    ///
    inline pb_t *PB(void) { return m_pb; }
    inline const pb_t &PB(void) const { return *m_pb; }

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
    /// @param[in] pb Protobuf data.
    /// @param[in] uid User ID.
    /// @param[in] pid Parent ID.
    /// @param[in] type Protobuf data type.
    /// @param[in] id Protobuf data ID.
    /// @param[in] idx Protobuf data index.
    ///
    template<class Type>
        static Type *AddPB(const Type &pb, oid_t uid, oid_t pid, int type, oid_t id, int idx) {
            pb_t *dst = FindPB(id, type);
            if (dst == NULL) {
                dst = E_NEW Type(pb);
            } else {
                dst->CopyFrom(pb);
            }
            IndexProto(dst, uid, pid, type, id, idx);
            return static_cast<Type *>(dst);
        }

    ///
    /// Clone protobuf data.
    /// @param[out] pb Protobuf data.
    /// @param[in] id Protobuf data ID.
    /// @param[in] type Protobuf data type.
    ///
    static bool ClonePB(pb_t *pb, oid_t id, int type);

    ///
    /// Remove protobuf data.
    /// @param[in] id Protobuf data ID.
    /// @param[in] recursive Recursive removing.
    ///
    static void DelPB(oid_t id, bool recursive);

    ///
    /// Find Object by ID.
    /// @param[in] id Object ID.
    /// @return Pointer to Object if found, or NULL.
    ///
    static Object *FindObject(oid_t id);

    ///
    /// Get parent ID.
    /// @param[in] id Proto ID.
    /// @return Parent ID.
    ///
    static oid_t GetPID(oid_t id);

    ///
    /// Find Proto by ID.
    /// @param[in] id Proto ID.
    /// @return Pointer to Proto object if found, or NULL.
    ///
    static Proto *FindProto(oid_t id);

    ///
    /// Find protobuf data by ID.
    /// @param[in] id Protobuf data ID.
    /// @param[in] type Protobuf data type.
    /// @return Pointer to protobuf data if found, or NULL.
    ///
    template<class Type>
    static Type *FindPB(oid_t id, int type) {
        return static_cast<Type *>(FindPB(id, type));
    }

    ///
    /// Find protobuf data by ID.
    /// @param[in] id Protobuf data ID.
    /// @param[in] type Protobuf data type.
    /// @return Pointer to protobuf data if found, or NULL.
    ///
    static pb_t *FindPB(oid_t id, int type);

    ///
    /// Get max type of parent ID in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @return Pointer to container if found, or NULL.
    ///
    static int GetMaxType(oid_t pid);

    ///
    /// Get max index of parent ID in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Pointer to container if found, or NULL.
    ///
    static int GetMaxIndex(oid_t pid, int type);

    ///
    /// Find children by parent ID and type in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Pointer to container if found, or NULL.
    ///
    static id_set *GetChildren(oid_t pid, int type);

    ///
    /// Find the last/only child object ID by parent ID and type in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Last/Only element object ID.
    ///
    static oid_t GetLastChild(oid_t pid, int type);

    ///
    /// Check if has child object ID in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] id Object ID.
    /// @return true if found, or false.
    ///
    static bool HasChild(oid_t pid, int type, oid_t id);

    ///
    /// Get size of children in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Size of children.
    ///
    static size_t ChildrenSize(oid_t pid, int type);

    ///
    /// Get whole container by parent ID and type in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Whole Container.
    ///
    static id_ismap *GetContainerItems(oid_t pid, int type);

    ///
    /// Get container item IDs by parent ID and type/index in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] idx Object index.
    /// @return Container item IDs.
    ///
    static id_set *GetContainerItems(oid_t pid, int type, int idx);

    ///
    /// Get container item by parent ID, type, index in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] idx Object index.
    /// @return Protobuf data if exist, or NULL.
    ///
    static pb_t *GetContainerItem(oid_t pid, int type, int idx);

    ///
    /// Insert container item ID into `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] idx Object index.
    /// @param[in] id Object ID.
    ///
    static void AddContainerItem(oid_t pid, int type, int idx, oid_t id);

    ///
    /// Delete container item ID from `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] idx Object index.
    /// @param[in] id Object ID.
    ///
    static void DelContainerItem(oid_t pid, int type, int idx, oid_t id);

    ///
    /// Set the only child object ID into `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] id Object ID.
    ///
    static void SetChild(oid_t pid, int type, oid_t id);

    ///
    /// Insert child object ID into `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] id Object ID.
    ///
    static void AddChild(oid_t pid, int type, oid_t id);

    ///
    /// Remove child object ID from `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @param[in] id Object ID.
    ///
    static void DelChild(oid_t pid, int type, oid_t id);

    ///
    /// Remove children by parent ID and type from `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type, clear all(free memory) if type is 0.
    ///
    static void DelChildren(oid_t pid, int type = 0);

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

    ///
    /// Statistic info.
    /// @param[in] args Current time.
    ///
    static bool Stat(void *args);

    virtual ~Object(void);

protected:
    ///
    /// Index Proto.
    /// @param[in] id Proto ID.
    /// @param[in] pb Protobuf data.
    /// @param[in] uid User ID.
    /// @param[in] pid Parent ID.
    /// @param[in] type Proto type.
    /// @param[in] id Proto ID.
    /// @param[in] idx Proto index.
    ///
    static void IndexProto(pb_t *pb, oid_t uid, oid_t pid, int type, oid_t id, int idx);

    ///
    /// Unindex Proto.
    /// @param[in] id Proto ID.
    /// @param[in] recursive Recursive removing.
    ///
    static void UnindexProto(oid_t id, bool recursive);

    typedef std::map<oid_t, Proto *> proto_map;

    Object();

    ///
    /// On initialization, insert Object into global map.
    ///
    virtual void OnInit(void);

    ///
    /// Get container ID by parent ID and type in `s_containers`.
    /// @param[in] pid Parent ID.
    /// @param[in] type Object type.
    /// @return Container ID.
    ///
    static oid_t GetContainer(oid_t pid, int type);

    /// Object ID
    oid_t m_id;

    /// User ID
    oid_t m_uid;

    /// parent ID
    oid_t m_pid;

    /// Object type
    int m_type;

    /// index
    int m_idx;

    /// short ID
    int m_sid;

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
    static id_lismap s_containers;
};
} // namespace elf

