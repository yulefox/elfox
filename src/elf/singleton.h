/*
 * Copyright (C) 2008-2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file singleton.h
 * @author Fox (yulefox at gmail.com)
 * @date 2008-11-19
 * @brief singleton based on Boost.
 *
 * Using Getinst(CLASS_NAME) within single thread applications,
 * Using GetMTinst(CLASS_NAME) within multithread applications.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_SINGLETON_H
#define ELF_SINGLETON_H

#include <elf/config.h>

#define get_inst(CLASS_NAME) \
    Elf::singleton<CLASS_NAME>::inst()

#define get_inst_mt(CLASS_NAME) \
    Elf::singleton_mt<CLASS_NAME>::inst()

namespace elf {
namespace utils {
template<typename T>
class singleton
{
public:
    static T& inst()
    {
        static T obj;
        return obj;
    }

private:
    singleton(void) {}
    ~singleton(void) {}
    singleton(const singleton&);
    singleton& operator=(const singleton&);
};

template<typename T>
class singleton_mt
{
public:
    static T& inst()
    {
        static T obj;
        creator.donothing();
        return obj;
    }

private:
    singleton_mt(void) {}
    ~singleton_mt(void) {}
    singleton_mt(const singleton_mt&);
    singleton_mt& operator=(const singleton_mt&);

    class creator
    {
    public:
        creator(void) { singleton_mt<T>::inst(); }
        inline void donothing(void) const {}
    };

    static creator creator;
};

template<typename T>
typename singleton_mt<T>::creator singleton_mt<T>::creator;
} // namespace utils
} // namespace elf

#endif /* !ELF_SINGLETON_H */

