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
/// Generate several random number in [min, max].
/// @param min Minimum number.
/// @param max Maximum number.
/// @param res Roll result.
/// @param times Roll times.
///
void rand(int min, int max, roll_set &res, int times);

///
/// Roll around group.
/// @param req Roll request.
/// @param res Roll response.
/// @param times Roll times.
void roll(const roll_req &req, roll_res &res, int times);
} // namespace elf

#endif /* !ELF_RAND_H */

