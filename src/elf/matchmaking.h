/*
 * Author: youngtrips
 * Created Time:  Fri 30 Oct 2015 04:41:06 PM CST
 * File Name: matchmaking.h
 * Description: 
 */

#ifndef __MATCHMAKING_H
#define __MATCHMAKING_H

#include <queue>
#include <map>

#include <elf/oid.h>

namespace elf {

typedef std::set<oid_t> TeamSet;

struct MatchEntity;
struct MatchRes;
struct MatchQueue;

class MatchPool {
public:
    struct MatchRes {
        int type;
        std::vector<TeamSet> teams;
        bool operator < (const MatchRes &res) {
            return type < res.type;
        }
    };

public:
    enum MatchStatus {
        MATCH_PENDING,  // waitting
        MATCH_DONE,     // matched
        MATCH_DELETED,  // deleted
    };
    enum MatchMode {
        MATCH_MOD_RAND, // rand match
        MATCH_MOD_ELO,  // match opponent with elo
    };

public:
    ~MatchPool();
    oid_t Push(int elo, int size, oid_t team_id);
    void Del(const oid_t &id);

private:
    MatchPool();
    MatchPool(int type, int mode, int team_size, int camp_size);
    bool tick(std::list<MatchRes> &resList);

    void push(MatchEntity *ent);

    bool pop(int size_type);
    bool pop(std::list<MatchRes> &resList);
    bool pop(int size_type, MatchRes &res);
    bool pop(int size_type, std::vector<MatchEntity*> &camps);

    bool get_opponent(MatchEntity *ent, MatchEntity *opp, int need_size);
    bool get_opponent_elo(MatchEntity *ent, MatchEntity *opp, int need_size);

    bool get_buddy(int need_size, MatchEntity *dst);
    bool get_buddy_elo(int need_size, MatchEntity *dst);
    bool get_buddy_rand(int need_size, MatchEntity *dst);

    bool get_opponent_rand(MatchEntity *ent, MatchEntity *opp, int need_size);
    bool get_entity_rand(int need_size, MatchEntity *dst);

    bool top(MatchEntity &ent, int size_type);

    MatchEntity *get_entity(const oid_t &id); 
    MatchEntity clone_entity(MatchEntity *src);

private:
    int _type;
    int _mode;
    int _team_size;
    int _camp_size;
    std::map<int, MatchQueue*> _queues;
    std::map<oid_t, MatchEntity*> _entities;

public:
    static MatchPool *Create(int type, int mode, int team_size, int camp_size);
    static MatchPool *Get(int type);
    static void Proc(std::list<MatchRes> &res);
    static void Release();

private:
    static std::map<int, MatchPool*> s_pools; // type ==> pool
};

} // namespace elf

#endif /* !__MATCHMAKING_H */
