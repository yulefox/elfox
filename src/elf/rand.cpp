/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/rand.h>
#include <math.h>

namespace elf {
float frand(float min, float max)
{
    float factor = fabs(max - min) / RAND_MAX;

    return (min + ::rand() * factor);
}

int rand(int min, int max)
{
    int range = abs(max - min) + 1;

    return (min + ::rand() % range);
}

int rand_hit(int range, int times)
{
    int hit = 0;

    for (int i = 0; i < times; ++i) {
        if (rand(1, 10000) <= range) {
            ++hit;
        }
    }
    return hit;
}

void roll_pb(int min, int max, roll_res &res, int times)
{
    roll_res::iterator itr_d;

    while (times--) {
        int n = rand(min, max);

        itr_d = res.find(n);
        if (itr_d == res.end()) {
            res[n] = 1;
        } else {
            ++(itr_d->second);
        }
    }
}

void roll_rm(int min, int max, roll_res &res, int times)
{
    int range = abs(max - min) + 1;

    assert(range >= times);
    if (range * 2 > times * 3) { // inc
        while (times) {
            int n = rand(min, max);

            if (res.find(n) == res.end()) {
                res[n] = 1;
                --times;
            }
        }
    } else { // dec
        for (int i = min; i <= max; ++i) {
            res[i] = 1;
        }
        times = range - times;
        while (times) {
            int n = rand(min, max);

            if (res.find(n) != res.end()) {
                res.erase(n);
                --times;
            }
        }
    }
}

void rand(const roll_req &req, roll_res &res, int times)
{
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;

    while (times--) {
        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            int rnd = rand(1, 10000);

            if (rnd <= itr_c->second) {
                itr_d = res.find(itr_c->first);
                if (itr_d == res.end()) {
                    res[itr_c->first] = 1;
                } else {
                    ++(itr_d->second);
                }
            }
        }
    }
}

void roll_pb(const roll_req &req, roll_res &res, int times, bool weight)
{
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;
    int weight_sum = 0;

    if (weight) {
        for (; itr_c != req.end(); ++itr_c) {
            weight_sum += itr_c->second;
        }
    } else {
        weight_sum = 10000;
    }

    while (times--) {
        int rnd = rand(1, weight_sum);

        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            if (rnd <= itr_c->second) { // hit
                itr_d = res.find(itr_c->first);
                if (itr_d == res.end()) {
                    res[itr_c->first] = 1;
                } else {
                    ++(itr_d->second);
                }
                break;
            }
            rnd -= itr_c->second;
        }
    }
}

void roll_rm(const roll_req &req, roll_res &res, int times, bool weight)
{
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;
    int weight_sum = 0;

    if (weight) {
        for (; itr_c != req.end(); ++itr_c) {
            weight_sum += itr_c->second;
        }
    } else {
        weight_sum = 10000;
    }

    while (times) {
        int rnd = rand(1, weight_sum);

        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            if (rnd <= itr_c->second) { // hit
                itr_d = res.find(itr_c->first);
                if (itr_d == res.end()) {
                    res[itr_c->first] = 1;
                    break;
                }
            }
            rnd -= itr_c->second;
        }
        --times;
    }
}

void rand_str(char *rnds, int len)
{
    static const int BASE_LEN = 62;
    static const char BASE[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        rnds[i] = BASE[::rand() % BASE_LEN];
    }
    rnds[len] = '\0';
}
} // namespace elf

