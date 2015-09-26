/**
 * Copyright (C) 2012-2015 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file match.h
 * @date 2012-09-22
 * @author Fox (yulefox at gmail.com)
 * Matchmaking.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_MATCH_H
#define ELF_MATCH_H

#include <elf/elf.h>
#include <elf/time.h>
#include <elf/oid.h>
#include <elf/elo.h>
#include <list>
#include <vector>

#define MAX_TEAM_MEMBER_SIZE    5
#define MAX_RANK_QUEUE_RANGE    100

#define GetRankManager() get_inst(elf::RankManager)
#define GetRankQueue() get_inst(elf::RankQueue)

namespace elf {
struct Rank {
    Rank()
    : _id(OID_NIL)
    , _exp_rank(0)
    {
        memset(&_rank, 0, sizeof(_rank));
    }

    ~Rank() {
    }

    oid_t _id;
    rank_t _rank;
    int _exp_rank;
};

typedef std::map<oid_t, Rank *> rank_map;
typedef std::vector<Rank *> rank_list;

class RankManager {
public:
    Rank *Add(oid_t id, int val);
    Rank *Get(oid_t id);

    inline int Size(void) const {
        return _ranks.size();
    }

    void Release(void);
    void Trace(void);

    rank_map _ranks;
};

class Team /* pre-team and gaming team */
{
public:
    Team(void);
    virtual ~Team(void);
    ELF_INL void Add(Rank *p);
    ELF_INL int GetRank(void) const;
    static Team *Create(Rank **p, int no = 1);
    static void Destroy(Team *t);

    friend class RankQueue;

private:
    int _member_num;
    time64_t _enter_time;
    Rank *_members[MAX_TEAM_MEMBER_SIZE];
};

class ELF_DATA RankQueue {
public:
    void Remove(oid_t &id, int no);
    void Init(void);
    void Release(void);
    void Push(Rank *p);
    void Push(Team *t);
    void Remove(Rank *p);
    void Pop(int no);
    void Pop(void);

    ELF_INL int PlayerNum(void) {
        return _player_num;
    }

    void Trace(void);

    typedef std::list<Team *> SubQueue;

private:
    int FrontQueue(void);

    int _player_num;
    int _sqn;
    SubQueue *_sqs;
};
} // namespace elf

#endif /* !ELF_MATCH_H */

