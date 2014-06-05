/*
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file event.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-01-10
 * Event module.
 */


#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_EVENT_H
#define ELF_EVENT_H

#include <elf/config.h>
#include <elf/oid.h>

namespace elf {
int event_init(void);
int event_fini(void);

///
/// Regist new event listener.
/// @param evt Event type.
/// @param cb Callback handle.
/// @param lid Listener id.
/// @param arg Listener argument.
///
void event_regist(int evt, callback_t *cb);

///
/// Unregist event listener.
/// @param lid Listener id.
/// @param evt Event type, unregist all about given listener if 0.
///
void event_unregist(oid_t lid, int evt = 0);

///
/// Emit event.
/// @param evt Event type.
/// @param arg Event argument.
/// @param oid Object id bound to the event.
///
void event_emit(int evt, int arg, oid_t oid);
} // namespace elf

#endif /* !ELF_EVENT_H */

