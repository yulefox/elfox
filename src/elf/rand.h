/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file rand.h
 * @author Fox (yulefox at gmail.com)
 * @date 2014-01-08
 * @brief RAND(int 64, long long) generation algorithm.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_RAND_H
#define ELF_RAND_H

#include <elf/config.h>
#include <set>
#include <map>

namespace elf {
typedef std::set<int> roll_set;
typedef std::map<int, int> roll_req;
typedef std::map<int, int> roll_res;

///
/// Generate float random number in [min, max].
/// @param min Minimum number.
/// @param max Maximum number.
/// @return Generated random number.
///
float frand(float min, float max);

///
/// Generate integer random number in [min, max].
/// @param min Minimum number.
/// @param max Maximum number.
/// @return Generated integer random number.
///
int rand(int min, int max);

///
/// Generate several random numbers in [min, max].
/// @param min Minimum number.
/// @param max Maximum number.
/// @param res Roll result.
/// @param times Roll times.
///
void roll(int min, int max, roll_set &res, int times);

///
/// Generate several random requests in request group.
/// @param req Random request gorup.
/// @param res Random response group.
/// @param times Random times.
///
void rand(const roll_req &req, roll_res &res, int times);

///
/// Generate several random requests in request group(put back if hit).
/// @param req Random request group.
/// @param res Random response gorup.
/// @param times Random times.
///
void roll_pb(const roll_req &req, roll_res &res, int times);

///
/// Generate several random requests in request group(remove if hit).
/// @param req Roll request group.
/// @param res Roll response group.
/// @param times Roll times.
///
void roll_rm(const roll_req &req, roll_res &res, int times);

///
/// Generate random string(CDKEY).
/// @param rnds Generated random string.
/// @param len String length.
///
void rand_str(char *rnds, int len);
} // namespace elf

#endif /* !ELF_RAND_H */

