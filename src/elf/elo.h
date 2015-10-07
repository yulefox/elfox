/**
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file elo.h
 * @date 2012-09-19
 * @author Fox (yulefox at gmail.com)
 * FIDE Elo Rating System for PVP games.
 */

#ifndef ELF_ELO_H
#define ELF_ELO_H

#include <elf/config.h>

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#define DEFAULT_RANK            1200
#define ROOKIE_RANK             800
#define MASTER_RANK             2500

enum game_result_t {
    GAME_RESULT_WIN,
    GAME_RESULT_DRAW,
    GAME_RESULT_LOSS,
    GAME_RESULT_UNDEFINED = -1,
};

struct elo_t {
    int R; /* current Elo rank */
    int K; /* K-factor */
    int W; /* won games */
    int D; /* drawn games, or not recorded lost games */
    int L; /* lost games */
};

/**
 * @param[out] lt left team
 * @param[in] lres result of left team
 * @param[out] rt right team
 * @param[in] rres result of right team
 * @param no number of team members
 */
ELF_API void elo_rating(elo_t **lt, int lres, elo_t **rt,
    int rres, int no);

ELF_API void elo_set(int *k);

#endif /* !ELF_ELO_H */

