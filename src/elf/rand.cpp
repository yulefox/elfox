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

void rand(int min, int max, roll_set &res, int times)
{
    int range = abs(max - min) + 1;

    assert(range >= times);
    if (range * 2 > times * 3) { // inc
        while (times--) {
            while (true) {
                int n = rand(min, max);

                if (res.find(n) == res.end()) {
                    res.insert(n);
                    break;
                }
            }
        }
    } else { // dec
        times = range - times;
        for (int i = min; i <= max; ++i) {
            res.insert(i);
        }
        while (times--) {
            while (true) {
                int n = rand(min, max);

                if (res.find(n) != res.end()) {
                    res.erase(n);
                    break;
                }
            }
        }
    }
}

void roll(const roll_req &req, roll_res &res, int times)
{
    roll_req::const_iterator itr_c = req.begin();
    roll_res::iterator itr_d;

    // rolling
    for (int i = 0; i < times; ++i) {
        int rnd = rand(0, 10000);

        for (itr_c = req.begin(); itr_c != req.end(); ++itr_c) {
            if (rnd <= itr_c->second) {
                itr_d = res.find(itr_c->first);
                if (itr_d == res.end()) {
                    res[itr_c->first] = 1;
                } else {
                    ++(itr_d->second);
                }
                break;
            }
        }
    }
}
} // namespace elf

