/*
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/event.h>
#include <elf/log.h>
#include <elf/memory.h>
#include <map>
#include <set>

namespace elf {
typedef std::list<callback_t *> callback_list;
typedef std::map<oid_t, callback_list *> listener_list;
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
    callback_list *cl = NULL;
    listener_list *ll = NULL;
    listener_map::iterator itr = s_listeners.find(evt);

    if (itr == s_listeners.end()) {
        ll = E_NEW listener_list;
        s_listeners[evt] = ll;
    } else {
        ll = itr->second;
    }

    listener_list::iterator itr_l = ll->find(cb->lid);

    if (itr_l == ll->end()) {
        cl = E_NEW callback_list;
        (*ll)[cb->lid] = cl;
    } else {
        cl = itr_l->second;
    }
    cb->busy = false;
    cl->push_back(cb);
    LOG_TRACE("event", "<%lld> regist event %d.",
            cb->lid, evt);
}

void event_unregist(oid_t lid, oid_t oid, int evt)
{
    LOG_TRACE("event", "<%lld> unregist event %d.",
            lid, evt);
    if (evt > 0) {
        listener_map::iterator itr = s_listeners.find(evt);

        if (itr == s_listeners.end()) {
            LOG_WARN("event", "<%lld> has NO event %d.",
                    lid, evt);
            return;
        }

        listener_list *ll = itr->second;

        assert(ll);
        listener_list::iterator itr_l = ll->find(lid);

        if (itr_l != ll->end()) {
            callback_list *cl = itr_l->second;
            callback_list::iterator itr_c = cl->begin();

            while (itr_c != cl->end()) {
                callback_t *cb = *itr_c;

                if (!(cb->busy) && (oid == OID_NIL || oid == cb->oid)) {
                    E_FREE(cb);
                    itr_c = cl->erase(itr_c);
                } else {
                    ++itr_c;
                }
            }
            if (cl->empty()) {
                E_DELETE(cl);
                ll->erase(itr_l);
            }
        }
    } else {
        listener_map::iterator itr = s_listeners.begin();

        for (; itr != s_listeners.end(); ++itr) {
            listener_list *ll = itr->second;

            assert(ll);
            listener_list::iterator itr_l = ll->find(lid);

            if (itr_l != ll->end()) {
                callback_list *cl = itr_l->second;
                callback_list::iterator itr_c = cl->begin();

                while (itr_c != cl->end()) {
                    callback_t *cb = *itr_c;

                    if (!(cb->busy) && (oid == OID_NIL || oid == cb->oid)) {
                        E_FREE(cb);
                        itr_c = cl->erase(itr_c);
                    } else {
                        ++itr_c;
                    }
                }
                if (cl->empty()) {
                    E_DELETE(cl);
                    ll->erase(itr_l);
                }
            }
        }
    }
}

void event_emit(int evt, int arg_a, int arg_b, oid_t lid)
{
    LOG_TRACE("event", "<%lld> emit event %d:%d(%d).",
            lid, evt, arg_a, arg_b);

    // get event listeners
    listener_map::iterator itr = s_listeners.find(evt);

    if (itr == s_listeners.end()) {
        return;
    }

    // get listener
    listener_list *ll = itr->second;
    listener_list::iterator itr_l = ll->find(lid);

    if (itr_l == ll->end()) {
        return;
    }

    callback_list *cl = itr_l->second;
    callback_list::iterator itr_c = cl->begin();

    while (itr_c != cl->end()) {
        callback_t *cb = *itr_c;

        cb->busy = true;
        cb->tid = lid;
        cb->targ_a = arg_a;
        cb->targ_b = arg_b;
        if (cb->func(cb)) {
            E_FREE(cb);
            cl->erase(itr_c++);
        } else {
            itr_c++;
        }
    }
    if (cl->empty()) {
        E_DELETE(cl);
        ll->erase(itr_l);
    }
}
} // namespace elf

