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

void roll(int min, int max, roll_set &res, int times)
{
    int range = abs(max - min) + 1;

    assert(range >= times);
    if (range * 2 > times * 3) { // inc
        while (times) {
            int n = rand(min, max);

            if (res.find(n) == res.end()) {
                res.insert(n);
                --times;
            }
        }
    } else { // dec
        for (int i = min; i <= max; ++i) {
            res.insert(i);
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

void roll_pb(const roll_req &req, roll_res &res, int times)
{
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;

    // rolling
    while (times--) {
        int rnd = rand(1, 10000);

        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            if (rnd <= itr_c->second) {
                itr_d = res.find(itr_c->first);
                if (itr_d == res.end()) {
                    res[itr_c->first] = 1;
                } else {
                    ++(itr_d->second);
                }
            }
            rnd -= itr_c->second;
        }
    }
}

void roll_rm(const roll_req &req, roll_res &res, int times)
{
    int range = req.size();

    assert(range >= times);
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;

    while (times) {
        int rnd = rand(1, 10000);

        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            if (rnd <= itr_c->second) {
                for (itr_d = res.begin(); itr_d != res.end(); ++itr_d) {
                    if (itr_d->second == itr_c->second) {
                        break;
                    }
                }
                if (itr_d == res.end()) {
                    res[--times] = itr_c->first;
                    break;
                }
            }
            rnd -= itr_c->second;
        }
    }
}

void rand_str(char *rnds, int len)
{
}
} // namespace elf

