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
#include <elf/oid.h>
#include <elf/pb.h>
#include <map>
#include <string>

namespace elf {
class Object;
typedef std::map<oid_t, Object *> obj_map_id;
typedef std::map<int, Object *> obj_map_int;
typedef std::map<std::string, Object *> obj_map_str;

class Object {
public:
    ///
    /// Get object id.
    /// @return Object id.
    ///
    inline oid_t GetID(void) const { return m_id; }

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
    /// Find object by id.
    /// @param id Object id.
    /// @return Pointer to object if found, or NULL.
    ///
    template<class Type>
    static Type *Find(oid_t id) {
        obj_map_id::const_iterator itr =s_objs.find(id);

        if (itr != s_objs.end()) {
            return static_cast<Type *>(itr->second);
        }
        return NULL;
    }

    ///
    /// Find object by id with dynamic_cast.
    /// @param id Object id.
    /// @return Pointer to object if found, or NULL.
    ///
    template<class Type>
    static Type *SafeFind(oid_t id) {
        obj_map_id::const_iterator itr =s_objs.find(id);

        if (itr != s_objs.end()) {
            return dynamic_cast<Type *>(itr->second);
        }
        return NULL;
    }

    ///
    /// Find PB by id.
    /// @param id Object id.
    /// @return Pointer to object if found, or NULL.
    ///
    template<class Type>
    static Type *FindPB(oid_t id) {
        obj_map_id::const_iterator itr =s_objs.find(id);

        if (itr != s_objs.end()) {
            return static_cast<Type *>(itr->second->m_pb);
        }
        return NULL;
    }

    ///
    /// Get size of object map.
    /// @return Size of object map.
    ///
    static int Size(void) { return s_objs.size(); }

    virtual ~Object(void);

protected:
    Object();
    Object(oid_t id);

    /// object id
    oid_t m_id;

    /// object id
    std::string m_name;

    /// pb data
    pb_t *m_pb;

    /// global objects map
    static obj_map_id s_objs;
};
} // namespace elf

