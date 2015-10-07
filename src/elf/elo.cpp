/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elo.h>
#include <elf/elf.h>
#include <math.h>

#define NOVICE_ROUND_NUM        10
#define PROTECTED_ROUND_NUM     50
#define BONUS_POINT             1
#define MAX_BONUS_POINT         10
#define ZOOM_RANK               50
#define Q(R) pow(10.0f, R / 400.0f)

ELF_INL static void calc_K(elo_t *e, int trank);
ELF_INL static float calc_E(int ltrank, int rtrank);
ELF_INL static int calc_R(elo_t **lt, int no);

static float SCORE[] = {1.0f, 0.5f, 0.0f};
static int K[] = {50, 30, 20, 10};

void elo_rating(elo_t **lt, int lres, elo_t **rt,
    int rres, int no)
{
    assert(lt && rt);
    int ltrank, rtrank; /* team rank */
    float ltexp, rtexp; /* expectation */

    ltrank = calc_R(lt, no);
    rtrank = calc_R(rt, no);
    ltexp = calc_E(ltrank, rtrank);
    rtexp = 1 - ltexp;
    for (int i = 0; i < no; ++i) {
        assert(lt[i] && rt[i]);
        calc_K(lt[i], ltrank);
        calc_K(rt[i], rtrank);
        lt[i]->R += (int)(lt[i]->K * (SCORE[lres] - ltexp))/* + BONUS_POINT*/;
        rt[i]->R += (int)(rt[i]->K * (SCORE[rres] - rtexp))/* + BONUS_POINT*/;
        if (lt[i]->R < 0)
            lt[i]->R = 0;
        if (rt[i]->R < 0)
            rt[i]->R = 0;
        assert(lt[i]->R < 10000);
        switch (lres) {
        case GAME_RESULT_WIN:
            ++lt[i]->W;
            break;
        case GAME_RESULT_DRAW:
            ++lt[i]->D;
            break;
        case GAME_RESULT_LOSS:
            ++lt[i]->L;
            break;
        default:
            LOG_ERROR("rating", "Invalid game result: %d.", lres);
            break;
        }
        switch (rres) {
        case GAME_RESULT_WIN:
            ++rt[i]->W;
            break;
        case GAME_RESULT_DRAW:
            ++rt[i]->D;
            break;
        case GAME_RESULT_LOSS:
            ++rt[i]->L;
            break;
        default:
            LOG_ERROR("rating", "Invalid game result: %d.", rres);
            break;
        }
    }
}

void elo_set(int *k)
{
    memcpy(K, k, sizeof(K));
}

static void calc_K(elo_t *e, int trank)
{
    if (e->W + e->L < NOVICE_ROUND_NUM) {
        e->K = K[0];
    } else if (e->W + e->L < PROTECTED_ROUND_NUM) {
        e->K = K[1];
    } else if (e->R > MASTER_RANK) {
        e->K = K[3];
    } else {
        e->K = K[2];
    }
    e->K += std::min((e->R - trank) / ZOOM_RANK, MAX_BONUS_POINT);
    assert(e->K < 100);
}

static float calc_E(int ltrank, int rtrank)
{
    float lq = Q(ltrank);
    float rq = Q(rtrank);
    return lq / (lq + rq);
}

static int calc_R(elo_t **t, int no)
{
    int sum = 0;

    for (int i = 0; i < no; ++i) {
        sum += t[i]->R;
    }
    return sum / no;
}
