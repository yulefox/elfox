/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/event.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <map>
#include <set>

namespace elf {
typedef std::map<oid_t, callback_t *> listener_list;
typedef std::map<int, listener_list *> listener_map;

static listener_map s_listeners;

int event_init(void)
{
    MODULE_IMPORT_SWITCH;
    return 0;
}

int event_fini(void)
{
    MODULE_IMPORT_SWITCH;
    return 0;
}

void event_regist(int evt, callback_t *cb)
{
    assert(cb);
    listener_list *lss = NULL;
    listener_map::iterator itr = s_listeners.find(evt);

    if (itr == s_listeners.end()) {
        lss = E_NEW listener_list;
        s_listeners[evt] = lss;
    } else {
        lss = itr->second;
    }
    (*lss)[cb->lid] = cb;
}

void event_unregist(oid_t lid, int evt)
{
    if (evt > 0) {
        listener_map::iterator itr = s_listeners.find(evt);

        if (itr == s_listeners.end()) {
            return;
        }

        listener_list *lss = itr->second;

        assert(lss);
        listener_list::iterator itr_l = lss->find(lid);

        if (itr_l != lss->end()) {
            E_DELETE(itr_l->second);
            lss->erase(itr_l);
        }
    } else {
        listener_map::iterator itr = s_listeners.begin();

        for (; itr != s_listeners.end(); ++itr) {
            listener_list *lss = itr->second;

            assert(lss);
            listener_list::iterator itr_l = lss->find(lid);

            if (itr_l != lss->end()) {
                E_DELETE(itr_l->second);
                lss->erase(itr_l);
            }
        }
    }
}

void event_emit(int evt, int arg, oid_t oid)
{
    listener_map::iterator itr_l = s_listeners.find(evt);

    if (itr_l == s_listeners.end()) {
        return;
    }

    listener_list *lss = itr_l->second;
    listener_list::iterator itr = lss->begin();
    for (; itr != lss->end(); ++itr) {
        callback_t *cb = itr->second;

        assert(cb);

        cb->tid = oid;
        cb->targ = arg;
        cb->func(cb);
    }
}
} // namespace elf

