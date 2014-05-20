/*
 * Copyright (C) 2011-2013 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file timer.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-01-10
 *
 * There is a slot storing timers won't be expired.
 * It is NOT multi-thread safe.
 */


#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_TIMER_H
#define ELF_TIMER_H

#include <elf/config.h>
#include <elf/oid.h>
#include <elf/time.h>

namespace elf {
int timer_init(void);
int timer_fini(void);

void timer_run(void);
void timer_stat(void);

///
/// Return size of remain timers.
///
int timer_size(void);

///
/// Create a new timer.
/// @param life Life time of the timer(ms).
/// @param func Script function name bound to the timer.
/// @return id of the timer.
///
const oid_t &timer_add(time64_t life, const char *func);

///
/// Create a new timer.
/// @param life Life time of the timer(ms).
/// @param func Callback function.
/// @param args Callback arguments.
/// @return id of the timer.
///
const oid_t &timer_add(time64_t life, callback func, void *args);

///
/// Add cycle timer, period of one minte.
/// @param func Callback function.
///
void timer_cycle(callback func);

///
/// Remove timer with given tid.
/// @param tid Timer identification.
///
void timer_remove(const oid_t &tid);

///
/// Pause all running timers.
/// @return None.
///
void timer_pause(void);

///
/// Resume all running timers.
/// @return None.
///
void timer_resume(void);

///
/// Pause timer with given tid.
/// @param tid Timer identification.
///
void timer_pause(const oid_t &tid);

///
/// Resume timer with given tid.
/// @param tid Timer identification.
///
void timer_resume(const oid_t &tid);

///
/// Set timer interval for running timers.
/// @param t The value of timer interval.
/// For test only.
///
void timer_interval(time64_t t);

///
/// Set the bucket number for each wheel.
/// @param no Bucket number.
/// @param l Wheel level.
/// @return None.
/// For test only.
///
void timer_bucket(unsigned char no, int l);
} // namespace elf

#endif /* !ELF_TIMER_H */

