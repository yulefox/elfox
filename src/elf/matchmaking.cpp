#include <elf/oid.h>
#include <elf/memory.h>
#include <elf/time.h>
#include <elf/matchmaking.h>

namespace elf {

    enum MatchRank {
        ROOKIE_RANK = 800,
        MASTER_RANK = 2500,
        RANK_OFFSET = 100,
    };

    int get_rank_level(int rank) {
        if (rank <= ROOKIE_RANK) {
            return ROOKIE_RANK;
        } else if (rank > MASTER_RANK) {
            return MASTER_RANK;
        }
        return rank - rank % RANK_OFFSET;
    }

    struct MatchEntity {
        oid_t id;
        int elo;
        int status;
        int size;
        oid_t team_id;
        time_t time;

        std::list<MatchEntity*> children;

        int get_size() {
            if (children.empty()) {
                return size;
            }
            int total = 0;
            std::list<MatchEntity*>::iterator itr;
            for (itr = children.begin(); itr != children.end(); ++itr) {
                total += (*itr)->get_size();
            }
            return total;
        }
    };

    struct MatchRes {
        int type;
        std::vector<TeamSet> teams;
    };

    typedef std::map<int, TeamSet> rank_map_t;
    struct MatchQueue {
        int size_type;
        std::queue<MatchEntity*> candidates;
        rank_map_t ranks;
    };

    std::map<int, MatchPool*> MatchPool::s_pools;

    MatchPool::MatchPool() 
        : _type(0)
        , _team_size(0)
        , _camp_size(0) {
    }

    MatchPool::MatchPool(int type, int team_size, int camp_size)
        : _type(type)
        , _team_size(team_size)
        , _camp_size(camp_size) {
    }

    MatchPool::~MatchPool() {
        for (std::map<oid_t, MatchEntity*>::iterator itr = _entities.begin();
                itr != _entities.end();
                ++itr) {
            E_DELETE itr->second;
        }
        _entities.clear();

        for (std::map<int, MatchQueue*>::iterator itr = _queues.begin();
                itr != _queues.end();
                ++ itr) {
            E_DELETE itr->second;
        }

        _queues.clear();
    }

    oid_t MatchPool::Push(int elo, int size, oid_t teamId) {
        MatchEntity *ent = E_NEW MatchEntity;
        ent->id = oid_gen();
        ent->elo = elo;
        ent->time = time_s();
        ent->status = MATCH_PENDING;
        ent->team_id = teamId;
        ent->size = size;

        push(ent);
        return ent->id;
    }

    void  MatchPool::push(MatchEntity *ent) {
        _entities[ent->id] = ent;

        std::map<int, MatchQueue*>::iterator itr = _queues.find(ent->size);
        MatchQueue *que;
        if (itr == _queues.end()) {
            que = E_NEW MatchQueue;
            que->size_type = ent->size;
            _queues[ent->size] = que;
        } else {
            que = itr->second;
        }
        que->candidates.push(ent);

        int lvl = get_rank_level(ent->elo);
        rank_map_t::iterator itr_rank = que->ranks.find(lvl);
        if (itr_rank == que->ranks.end()) {
            que->ranks.insert(std::make_pair(lvl, TeamSet()));
        }
        que->ranks[lvl].insert(ent->id);
    }


    MatchEntity *MatchPool::top(int size_type) {
        std::map<int, MatchQueue*>::iterator itr = _queues.find(size_type);
        if (itr == _queues.end()) {
            return NULL;
        }
        MatchQueue *que = itr->second;
        if (que->candidates.empty()) {
            return NULL;
        }
        MatchEntity *ent = que->candidates.front();
        que->candidates.pop();
        return ent;
    }

    /// find skill proper opponent
    MatchEntity *MatchPool::get_opponent(MatchEntity *ent) {
        int lvl = get_rank_level(ent->elo);
        std::map<int, MatchQueue*>::iterator itr = _queues.find(ent->get_size());
        if (itr == _queues.end()) {
            return NULL;
        }
        MatchQueue *que = itr->second;

        rank_map_t::iterator itr_rank = que->ranks.find(lvl);
        if (itr_rank == que->ranks.end()) {
            return NULL;
        }
        TeamSet::iterator itr_t = itr_rank->second.begin();
        for (;itr_t != itr_rank->second.end(); ++itr_t) {
            if (*itr_t != ent->id) {
                return _entities[*itr_t];
            }
        }
        return NULL;
    }

    bool MatchPool::pop(int size_type) {
        std::vector<std::vector<MatchEntity*> > camps(_camp_size);

        MatchEntity *ent = top(size_type);
        if (ent == NULL) {
            return false;
        }
        camps[0].push_back(ent);

        for (int i = 1;i < _camp_size; i++) {
            MatchEntity *opp = get_opponent(ent);
            if (opp == NULL) {
                return false;
            }
            camps[i].push_back(opp);
        }

        int size = ent->get_size();
        while (size < _camp_size) {
            MatchEntity *buddy = NULL; //get_buddy(_camp_size - size);
            if (buddy == NULL) {
                return false;
            }
            camps[0].push_back(buddy);
            for (int i = 1;i < _camp_size; i++) {
                MatchEntity *opp_buddy = get_opponent(buddy);
                if (opp_buddy == NULL) {
                    return false;
                }
                camps[i].push_back(opp_buddy);
            }
            size += buddy->get_size();
            return true;
        }
        return false;
    }

    /// pop matched team(s)
    bool MatchPool::pop(MatchRes &res) {
        std::vector<std::vector<MatchEntity*> > camps(_camp_size);

        for (int i = _team_size;i >= 1; i--) {
            pop(i);
        }

        /*
        MatchEntity *ent = top();
        if (ent == NULL) {
            return false;
        }

        camps[0].push_back(ent);

        MatchEntity *opt = get_opponent(ent);
        if (opt == NULL) {
            return false;
        }
        */

        return false;
    }

    void MatchPool::Del(const oid_t &id) {
        std::map<oid_t, MatchEntity*>::iterator itr = _entities.find(id);
        if (itr != _entities.end()) {
            MatchEntity *ent = itr->second;
            ent->status = MatchPool::MATCH_DELETED;
        }
    }

    MatchPool *MatchPool::Create(int type, int team_size, int camp_size) {
        MatchPool *pool = Get(type);
        if (pool != NULL) {
            return pool;
        }
        pool = E_NEW MatchPool(type, team_size, camp_size);
        s_pools[type] = pool;
        return pool;
    }

    MatchPool *MatchPool::Get(int type) {
        std::map<int, MatchPool*>::iterator itr = s_pools.find(type);
        MatchPool *pool = NULL;
        if (itr != s_pools.end()) {
            pool = itr->second;
        }
        return pool;
    }

    void MatchPool::Proc(std::list<MatchRes> &resList) {
        std::map<int, MatchPool*>::iterator itr;
        for (itr = s_pools.begin(); itr != s_pools.end(); ++itr) {
            MatchRes res;
            if (itr->second->pop(res)) {
                resList.push_back(res);
            }
        }
    }

    void MatchPool::Release() {
        std::map<int, MatchPool*>::iterator itr;
        for (itr = s_pools.begin(); itr != s_pools.end(); ++itr) {
            E_DELETE itr->second;
        }
        s_pools.clear();
    }

} // namespace MatchMaking
