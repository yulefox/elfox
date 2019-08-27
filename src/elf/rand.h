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
typedef std::map<int, int> roll_req; // map<val, rate>
typedef std::map<int, int> roll_res; // map<val, num>

///
/// Set seed for a new sequence of pseudo-random integers to be returned by rand().
/// @param seed pseudo-random seed.
///
void srand(int seed);

///
/// Generate a pseudo-random intege in rthe mathematical range [0, RAND_MAX].
/// @return A pseudo-random integer.
///
int rand();

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
/// Hit times within range.
/// @param range Target range [0, 10000].
/// @param times Run times.
/// @return Hit times.
///
int rand_hit(int range, int times);

///
/// Generate several random numbers in [min, max](put back if hit).
/// @param min Minimum number.
/// @param max Maximum number.
/// @param res Roll result.
/// @param times Roll times.
///
void roll_pb(int min, int max, roll_res &res, int times);

///
/// Generate several random numbers in [min, max](remove if hit).
/// @param min Minimum number.
/// @param max Maximum number.
/// @param res Roll result.
/// @param times Roll times.
///
void roll_rm(int min, int max, roll_res &res, int times);

///
/// Generate several random requests in request group(independent).
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
/// @param weight With weight if true.
///
void roll_pb(const roll_req &req, roll_res &res, int times,
        bool weight = false);

///
/// Generate several random requests in request group(remove if hit).
/// @param req Roll request group.
/// @param res Roll response group.
/// @param times Roll times.
/// @param weight With weight if true.
///
void roll_rm(roll_req &req, roll_res &res, int times,
        bool weight = false);

///
/// Generate key after given times in request group(remove if hit).
/// @param req Roll request group.
/// @param times Roll times.
/// @param weight With weight if true.
/// @return Rand key after given times.
///
int roll_rm(roll_req &req, int times, bool weight = false);

///
/// Generate random string.
/// @param rnds Generated random string.
/// @param len String length.
///
void rand_str(char *rnds, int len);

///
/// Shuffle cards.
/// @param seed Random seed.
/// @param num Card num.
/// @param res Roll response group.
///
void shuffle_cards(unsigned int seed, int num, int *res);
} // namespace elf

#endif /* !ELF_RAND_H */

