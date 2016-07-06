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
    if (min == max) {
        return min;
    }

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

            if (res.erase(n) > 0) {
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

    assert(req.size() >= (size_t)times);

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
                    --times;
                    break;
                }
            }
            rnd -= itr_c->second;
        }
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

void shuffle_cards(unsigned int seed, int num, roll_res &res)
{
    unsigned int sp = seed;

    for (int i = 0; i < num; ++i) {
        int r = int(rand_r(&sp) % num);
        roll_res::iterator itr_i = res.find(i);
        roll_res::iterator itr_r = res.find(r);
        int t = 0;

        if (itr_i == res.end()) {
            itr_i = res.insert(std::make_pair(i, i)).first;
        }
        if (itr_r == res.end()) {
            itr_r = res.insert(std::make_pair(r, r)).first;
        }
        t = itr_i->second;
        itr_i->second = itr_r->second;
        itr_r->second = t;
    }
}
} // namespace elf

