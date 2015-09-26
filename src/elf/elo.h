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

BEGIN_C_DECLS

#define DEFAULT_RANK            1200
#define ROOKIE_RANK             800
#define MASTER_RANK             2500

typedef enum {
    GAME_RESULT_WIN,
    GAME_RESULT_DRAW,
    GAME_RESULT_LOSS,
    GAME_RESULT_UNDEFINED = -1,
} elo_result_t;

struct rank_t {
    int R; /* current Elo rank */
    int K; /* K-factor */
    int W; /* won games */
    int D; /* drawn games, or not recorded lost games */
    int L; /* lost games */
};

typedef struct rank_t rank_t;

/**
 * @param lt left team
 * @param rt right team
 * @param no number of team members
 */
ELF_API void elo_rating(rank_t **lt, int lres, rank_t **rt,
    int rres, int no);

ELF_API void elo_set(int *k);

END_C_DECLS

#endif /* !ELF_ELO_H */

