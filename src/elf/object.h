/*
 * Copyright (C) 2013-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file object.h
 * @author Fox(yulefox@gmail.com)
 * @date 2013-11-01
 * Object base.
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
class PBRef;

typedef std::list<Object *> obj_list;
typedef std::map<oid_t, Object *> obj_map_id;
typedef std::map<int, Object *> obj_map_int;
typedef std::map<std::string, Object *> obj_map_str;
typedef std::map<oid_t, pb_t *> pb_map_id;

class Object {
public:
    ///
    /// Get object id.
    /// @return Object id.
    ///
    inline oid_t GetID(void) const { return m_id; }

    ///
    /// Get object short ID.
    /// @return Guild short ID.
    ///
    inline int SID(void) const { return m_sid; }

    ///
    /// Get object name.
    /// @return Object name.
    ///
    inline const std::string &GetName(void) const { return m_name; }

    ///
    /// Get protobuf data.
    /// @return Protobuf data.
    ///
    template<class Type>
    inline Type *GetPB(void) {
        assert(m_pb);
        return static_cast<Type *>(m_pb);
    }

    ///
    /// Get protobuf data.
    /// @return Protobuf data.
    ///
    template<class Type>
    inline const Type &GetPB(void) const {
        assert(m_pb);
        return *(static_cast<Type *>(m_pb));
    }

    ///
    /// On initialization, insert object into global map.
    ///
    void OnInit(void);

    ///
    /// Output statistics info.
    ///
    static void Stat(void);


    ///
    /// Release all Object/protobuf objects.
    ///
    static void Release(void);

    ///
    /// Add protobuf object.
    /// @param pb protobuf object.
    /// @param id protobuf object id.
    ///
    template<class Type>
        static Type *AddPB(const Type &pb, oid_t id, int ref) {
            PBRef *pr = FindRef(id);
            Type *dst = NULL;

            if (pr == NULL) {
                pr = E_NEW PBRef;
                pr->pb = dst = E_NEW Type(pb);
                pr->ref = ref;
                s_pbs[id] = pr;
            } else {
                dst = static_cast<Type *>(pr->pb);
                if (dst != &pb) {
                    dst->CopyFrom(pb);
                }
            }
            return dst;
        }

    ///
    /// Remove protobuf object.
    /// @param id protobuf object id.
    ///
    static void DelPB(oid_t id);
    ///
    /// Clone protobuf object.
    /// @param pb protobuf object.
    /// @param id protobuf object id.
    ///
    template<class Type>
        static bool ClonePB(pb_t *pb, oid_t id) {
            assert(pb);

            Type *src = FindPB<Type>(id);

            if (src == NULL) {
                return false;
            }
            pb->CopyFrom(*src);
            return true;
        }

    ///
    /// Find object by id.
    /// @param id Object id.
    /// @return Pointer to object if found, or NULL.
    ///
    template<class Type>
        static Type *Find(oid_t id) {
            if (id == elf::OID_NIL) {
                return NULL;
            }

            obj_map_id::const_iterator itr =s_objs.find(id);

            if (itr != s_objs.end()) {
                return static_cast<Type *>(itr->second);
            }
            return NULL;
        }

    ///
    /// Find object by id with dynamic_cast.
    /// @param id Object id.
    /// @return Pointer to Object if found, or NULL.
    ///
    template<class Type>
        static Type *SafeFind(oid_t id) {
            if (id == elf::OID_NIL) {
                return NULL;
            }

            obj_map_id::const_iterator itr =s_objs.find(id);

            if (itr != s_objs.end()) {
                return dynamic_cast<Type *>(itr->second);
            }
            return NULL;
        }

    ///
    /// Find PB by id.
    /// @param id Object id.
    /// @return Pointer to pb_t object if found, or NULL.
    ///
    template<class Type>
        static Type *FindPB(oid_t id) {
            if (id == elf::OID_NIL) {
                return NULL;
            }

            pbref_map_id::const_iterator itr =s_pbs.find(id);

            if (itr != s_pbs.end()) {
                return static_cast<Type *>(itr->second->pb);
            }
            return NULL;
        }

    ///
    /// Get size of object map.
    /// @return Size of object map.
    ///
    static int Size(void) { return s_objs.size(); }

    ///
    /// Get size of protobuf map.
    /// @return Size of object map.
    ///
    static int SizePB(void) { return s_pbs.size(); }

    virtual ~Object(void);

protected:
    struct PBRef {
        pb_t *pb;
        int ref;
    };

    ///
    /// Find PBRef by id.
    /// @param id Object id.
    /// @return Pointer to PBRef object if found, or NULL.
    ///
    static PBRef *FindRef(oid_t id);

    typedef std::map<oid_t, PBRef *> pbref_map_id;

    Object();
    Object(oid_t id);

    /// object id
    oid_t m_id;

    /// parent id
    oid_t m_pid;

    /// short id
    int m_sid;

    /// object name
    std::string m_name;

    /// pb data
    pb_t *m_pb;

    /// global Object map
    static obj_map_id s_objs;

    /// global protobuf map
    static pbref_map_id s_pbs;

    /// global protobuf map
    static pb_map_int s_pbs_i;
};
} // namespace elf

