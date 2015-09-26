/*
 * Copyright (C) 2012-2015 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/match.h>
#include <elf/elf.h>
#include <algorithm>
#include <vector>

namespace elf {
#define NEIGHBOR(sqn, range, no) \
        ((sqn / MAX_TEAM_MEMBER_SIZE + range) * MAX_TEAM_MEMBER_SIZE + no)

struct player_rank_cmp {
    bool operator()(const Rank *l, const Rank *r) {
        ELF_ASSERT(l && r);
        if (l->_rank.R < r->_rank.R)
            return true;
        return false;
    }
};

static int Balance(rank_list &ps, rank_t **ts);
static int GenGameResult(rank_list &ps);

Rank *RankManager::Add(oid_t id, int val)
{
    Rank *p = E_NEW Rank;

    p->_rank.R = val;
    p->_exp_rank = 0;
    _ranks[id] = p;
    return p;
}

Rank *RankManager::Get(oid_t id)
{
    rank_map::iterator itr = _ranks.find(id);

    if (itr != _ranks.end())
        return itr->second;
    return NULL;
}

void RankManager::Release(void)
{
    rank_map::iterator itr = _ranks.begin();

    for (; itr != _ranks.end(); ++itr) {
        E_DELETE(itr->second);
    }
    _ranks.clear();
}

void RankManager::Trace(void)
{
    rank_map::iterator itr = _ranks.begin();

    for (; itr != _ranks.end(); ++itr) {
        Rank *p = itr->second;

        LOG_TEST("%s [E %4d/R %4d](W %d/D %d/L %d)",
            p->_id, p->_exp_rank,
            p->_rank.R, p->_rank.W, p->_rank.D, p->_rank.L);
    }
}

Team::Team(void)
    : _member_num(0)
    , _enter_time(0) {
}

Team::~Team(void) {
}

void Team::Add(Rank *p) {
    ELF_ASSERT(_member_num < MAX_TEAM_MEMBER_SIZE);
    _members[_member_num++] = p;
}

int Team::GetRank(void) const {
    int avr = 0;

    ELF_ASSERT(_member_num > 0);
    for (int i = 0; i < _member_num; ++i) {
        avr += _members[i]->_rank.R;
    }
    return (avr / _member_num);
}

Team *Team::Create(Rank **p, int no) {
    Team *t = E_NEW Team;

    for (int i = 0; i < no; ++i) {
        t->Add(*p);
    }
    return t;
}

void RankQueue::Init(void) {
    _player_num = 0;
    _sqn = MAX_TEAM_MEMBER_SIZE *
        ((MASTER_RANK - ROOKIE_RANK) / MAX_RANK_QUEUE_RANGE + 2);
    _sqs = E_NEW SubQueue[_sqn];
}

void RankQueue::Release(void) {
    SubQueue::iterator itr;

    for (int i = 0; i < _sqn; ++i) {
        for (itr = _sqs[i].begin(); itr != _sqs[i].end(); ++itr) {
            E_DELETE(*itr);
        }
    }
    delete[] _sqs;
}

void RankQueue::Push(Rank *p) {
    Team *t = Team::Create(&p);
    Push(t);
}

void RankQueue::Push(Team *t) {
    int n = 0;
    int r = t->GetRank();

    t->_enter_time = time_ms();
    if (r < ROOKIE_RANK) {
        n = 0;
    } else if (r >= MASTER_RANK) {
        n = _sqn / MAX_TEAM_MEMBER_SIZE - 1;
    } else {
        n = (r - ROOKIE_RANK) / MAX_RANK_QUEUE_RANGE;
    }
    n = (n * MAX_TEAM_MEMBER_SIZE) + t->_member_num - 1;
    _sqs[n].push_back(t);
    _player_num += t->_member_num;
    ELF_ASSERT(_player_num <= GetRankManager().Size());
}

void RankQueue::Remove(Rank *p) {
    int n = 0;

    if (p->_rank.R < ROOKIE_RANK) {
        n = 0;
    } else if (p->_rank.R >= MASTER_RANK) {
        n = _sqn - 1;
    } else {
        n = (p->_rank.R - ROOKIE_RANK) / MAX_RANK_QUEUE_RANGE;
    }
}

void RankQueue::Pop(int no) {
    ELF_ASSERT(no <= MAX_TEAM_MEMBER_SIZE);
    ELF_ASSERT(_player_num >= no * 2);

    Team *t;
    rank_list ps;
    SubQueue::iterator itr;
    rank_t *ts[MAX_TEAM_MEMBER_SIZE * 2];

    int pn = 0; /* players number */
    int ls = 1; /* left team score */
    int fn = FrontQueue(); /* front queue number */
    int n = fn, rn = 0, on = 0;

    while (n >= -_sqn && n < 2 * _sqn) {
        itr = _sqs[n].begin();
        while ((pn < no * 2) && itr != _sqs[n].end()) {
            t = *itr;
            ELF_ASSERT(t);
            for (int i = 0; i < t->_member_num; ++i) {
                ps.push_back(t->_members[i]);
            }
            pn += t->_member_num;
            E_DELETE(t);
            itr = _sqs[n].erase(itr);
        }

        if (pn >= no * 2) break;
        while (n >= -_sqn && n < 2 * _sqn) {
            if (rn > 0) {
                rn = -rn;
            } else {
                rn = -rn;
                ++rn;
            }
            n = NEIGHBOR(fn, rn, on);
            if (n >= 0 && n < _sqn && !_sqs[n].empty()) {
                break;
            }
        }
    }
    ELF_ASSERT(pn == no * 2);
    _player_num -= pn;
    ls = Balance(ps, ts);
    elo_rating(ts, ls, ts + no, 2 - ls, no);
}

void RankQueue::Trace(void) {

    for (int i = 0; i < _sqn; ++i) {
        SubQueue::iterator itrt;

        for (itrt = _sqs[i].begin(); itrt != _sqs[i].end(); ++itrt) {
            Team * t = *itrt;
            Rank *p;

            for (int j = 0; j < t->_member_num; ++j) {
                p = t->_members[i];
            }
            LOG_INFO("rating", "%lld [E %4d/R %4d](W %d/D %d/L %d)",
                p->_id, p->_exp_rank,
                p->_rank.R, p->_rank.W, p->_rank.D, p->_rank.L);
        }
    }
}

int RankQueue::FrontQueue(void) {
    int n = 0;
    time64_t t = time_ms() + 1000000;
    SubQueue::iterator itr;

    for (int i = 0; i < _sqn; ++i) {
        itr = _sqs[i].begin();
        if (itr != _sqs[i].end() && (*itr)->_enter_time < t) {
            t =(*itr)->_enter_time;
            n = i;
        }
    }
    if (_sqs[n].empty()) {
        LOG_WARN("rating", "Invalid queue (%d).", n);
    }
    return n;
}

static int Balance(rank_list &ps, rank_t **ts) {
    int no = ps.size();
    rank_list::iterator itr = ps.begin();
    rank_list lps, rps;
    rank_t **lts = ts;
    rank_t **rts = ts + no / 2;

    std::sort(ps.begin(), ps.end(), player_rank_cmp());
    for (int i = 0; i < no; ++i) {
        if (i % 4 == 1 || i % 4 == 2) {
            lps.push_back(ps[i]);
            *lts++ = &(ps[i]->_rank);
        } else {
            rps.push_back(ps[i]);
            *rts++ = &(ps[i]->_rank);
        }
    }
    ps.clear();
    ps.insert(ps.end(), lps.begin(), lps.end());
    ps.insert(ps.end(), rps.begin(), rps.end());
    return GenGameResult(ps);
}

static int GenGameResult(rank_list &ps) {
    //int times = 0;
    int lr = 0, rr = 0; /* rank of two teams */
    int ls = GAME_RESULT_WIN;
    rank_list::iterator itr;

    int i = 0, no = ps.size() / 2;
    for (itr = ps.begin(); itr != ps.end(); ++i, ++itr) {
        Rank *p = *itr;

        if (i < no)
            lr += p->_exp_rank * p->_exp_rank;
        else
            rr += p->_exp_rank * p->_exp_rank;
    }

    if (lr < rr)
        ls = GAME_RESULT_LOSS;
    return ls;
}
} // namespace elf

