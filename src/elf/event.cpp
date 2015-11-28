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
typedef std::map<oid_t, callback_t *> callback_list;
typedef std::map<oid_t, callback_list *> listener_list;
typedef std::map<int, listener_list *> listener_map;

static listener_map s_listeners;

enum event_oper {
    EVENT_OPER_REGIST,
    EVENT_OPER_UNREGIST,
    EVENT_OPER_EMIT,
};

struct event_oper_t {
    int oper;
    int evt;
    int arg_a;
    int arg_b;
    oid_t oid;
    oid_t lid;
    callback_t *cb;

    event_oper_t(int e, callback_t *c) :
        oper(EVENT_OPER_REGIST),
        evt(e),
        cb(c)
    {
    }

    event_oper_t(int e, oid_t o, oid_t l) :
        oper(EVENT_OPER_UNREGIST),
        evt(e),
        oid(o),
        lid(l)
    {
    }

    event_oper_t(int e, int a, int b, oid_t o) :
        oper(EVENT_OPER_EMIT),
        evt(e),
        arg_a(a),
        arg_b(b),
        oid(o)
    {
    }
};

static std::list<event_oper_t *> s_ops;

static void regist(int evt, callback_t *cb)
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

    listener_list::iterator itr_l = ll->find(cb->oid);
    callback_list::iterator itr_c;

    if (itr_l == ll->end()) {
        cl = E_NEW callback_list;
        (*ll)[cb->oid] = cl;
    } else {
        cl = itr_l->second;
    }
    itr_c = cl->find(cb->lid);
    if (itr_c != cl->end()) {
        LOG_WARN("event", "<%lld><%lld> regist event %d (%d) ALREADY.",
                cb->oid, cb->lid, evt, cb->larg);
    } else {
        (*cl)[cb->lid] = cb;
        LOG_TRACE("event", "<%lld><%lld> regist event %d (%d).",
                cb->oid, cb->lid, evt, cb->larg);
    }
}

static void unregist(int evt, oid_t oid, oid_t lid)
{
    LOG_TRACE("event", "<%lld> <%lld> unregist event %d.",
            oid, lid, evt);
    if (evt > 0) {
        listener_map::iterator itr = s_listeners.find(evt);

        if (itr == s_listeners.end()) {
            LOG_WARN("event", "<%lld> has NO event %d.",
                    oid, evt);
            return;
        }

        listener_list *ll = itr->second;

        assert(ll);
        listener_list::iterator itr_l = ll->find(oid);

        if (itr_l != ll->end()) {
            callback_list *cl = itr_l->second;
            callback_list::iterator itr_c = cl->begin();

            while (itr_c != cl->end()) {
                callback_t *cb = itr_c->second;

                if (lid == OID_NIL || lid == cb->lid) {
                    E_FREE(cb);
                    cl->erase(itr_c++);
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
            listener_list::iterator itr_l = ll->find(oid);

            if (itr_l != ll->end()) {
                callback_list *cl = itr_l->second;
                callback_list::iterator itr_c = cl->begin();

                while (itr_c != cl->end()) {
                    callback_t *cb = itr_c->second;

                    if (lid == OID_NIL || lid == cb->lid) {
                        E_FREE(cb);
                        cl->erase(itr_c++);
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

static void emit(int evt, int arg_a, int arg_b, oid_t oid)
{
    LOG_TRACE("event", "<%lld> emit event %d:%d(%d).",
            oid, evt, arg_a, arg_b);

    // get event listeners
    listener_map::iterator itr = s_listeners.find(evt);

    if (itr == s_listeners.end()) {
        return;
    }

    // get listener
    listener_list *ll = itr->second;
    listener_list::iterator itr_l = ll->find(oid);

    if (itr_l == ll->end()) {
        return;
    }

    callback_list *cl = itr_l->second;
    callback_list::iterator itr_c = cl->begin();

    for (; itr_c != cl->end(); ++itr_c) {
        callback_t *cb = itr_c->second;

        cb->tid = oid;
        cb->targ_a = arg_a;
        cb->targ_b = arg_b;
        cb->func(cb);
    }
}

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

int event_proc(void)
{
    std::list<event_oper_t *> ops;

    s_ops.swap(ops);

    std::list<event_oper_t *>::const_iterator itr = ops.begin();
    for (; itr != ops.end(); ++itr) {
        event_oper_t *op = *itr;

        switch (op->oper) {
            case EVENT_OPER_REGIST:
                regist(op->evt, op->cb);
                break;
            case EVENT_OPER_UNREGIST:
                unregist(op->evt, op->oid, op->lid);
                break;
            case EVENT_OPER_EMIT:
                emit(op->evt, op->arg_a, op->arg_b, op->oid);
                break;
            default:
                assert(0);
                break;
        }
        E_DELETE(op);
    }
    return 0;
}

void event_regist(int evt, callback_t *cb)
{
    event_oper_t *op = E_NEW event_oper_t(evt, cb);

    s_ops.push_back(op);
}

void event_unregist(oid_t oid, oid_t lid, int evt)
{
    event_oper_t *op = E_NEW event_oper_t(evt, oid, lid);

    s_ops.push_back(op);
}

void event_emit(int evt, int arg_a, int arg_b, oid_t oid)
{
    event_oper_t *op = E_NEW event_oper_t(evt, arg_a, arg_b, oid);

    s_ops.push_back(op);
}
} // namespace elf

