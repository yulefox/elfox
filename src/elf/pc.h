/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file pc.h
 * @author Fox (yulefox at gmail.com)
 * @date 2014-01-09
 * @brief Producer-Consumer module.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_PC_H
#define ELF_PC_H

#include <elf/config.h>
#include <pthread.h>
#include <deque>

namespace elf {
template<class type>
class xqueue {
public:
    xqueue(): _nready(0) {
        pthread_mutex_init(&_ready, NULL);
        pthread_mutex_init(&_mutex, NULL);
        pthread_cond_init(&_cond, NULL);
    }

    size_t size(void) {
        return _queue.size();
    }

    int push(type &d) {
        pthread_mutex_lock(&_mutex);
        _queue.push_back(d);
        pthread_mutex_unlock(&_mutex);

        pthread_mutex_lock(&_ready);
        if (_nready == 0) {
            pthread_cond_signal(&_cond);
        }
        ++_nready;
        pthread_mutex_unlock(&_ready);
        return 0;
    }

    int pop(type &d) {
        pthread_mutex_lock(&_ready);
        if (_nready == 0) {
            pthread_cond_wait(&_cond, &_ready);
        }
        --_nready;
        pthread_mutex_unlock(&_ready);

        pthread_mutex_lock(&_mutex);
        d = _queue.front();
        _queue.pop_front();
        pthread_mutex_unlock(&_mutex);
        return 0;
    }

    int swap(std::deque<type> &clone) {
        pthread_mutex_lock(&_ready);
        _nready = 0;
        pthread_mutex_unlock(&_ready);

        pthread_mutex_lock(&_mutex);
        _queue.swap(clone);
        pthread_mutex_unlock(&_mutex);
        return 0;
    }

private:
    int _size;
    int _nready;
    pthread_mutex_t _ready;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
    std::deque<type> _queue;
};
} // namespace elf

#endif /* !ELF_PC_H */

