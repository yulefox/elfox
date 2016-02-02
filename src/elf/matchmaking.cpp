#include <elf/oid.h>
#include <elf/memory.h>
#include <elf/time.h>
#include <elf/rand.h>
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
        int camp;
        time_t time;

        std::list<MatchEntity> members;

        int get_size() {
            if (members.empty()) {
                return size;
            }
            int total = 0;
            std::list<MatchEntity>::iterator itr;
            for (itr = members.begin(); itr != members.end(); ++itr) {
                total += (*itr).get_size();
            }
            return total;
        }

        void get_members(TeamSet &memset) {
            if (members.empty()) {
                memset.insert(id);
                return;
            }
            std::list<MatchEntity>::iterator itr;
            for (itr = members.begin(); itr != members.end(); ++itr) {
                (*itr).get_members(memset);
            }
        }

        ~MatchEntity() {}
    };

    typedef std::map<int, TeamSet> rank_map_t;
    struct MatchQueue {
        int size_type;
        std::deque<oid_t> candidates;
        rank_map_t ranks;
    };

    std::map<int, MatchPool*> MatchPool::s_pools;

    MatchPool::MatchPool() 
        : _type(0)
        , _team_size(0)
        , _camp_size(0) {
    }

    MatchPool::MatchPool(int type, int mode, int team_size, int camp_size)
        : _type(type)
        , _mode(mode)
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

    MatchEntity *MatchPool::get_entity(const oid_t &id) {
        std::map<oid_t, MatchEntity*>::iterator itr = _entities.find(id);
        if (itr == _entities.end()) {
            return NULL;
        }
        return itr->second;
    }

    MatchEntity MatchPool::clone_entity(MatchEntity *src) {
        MatchEntity dst;
        dst = *src;
        return dst;
    }

    bool MatchPool::Push(int elo, int size, oid_t id) {
        MatchEntity *ent = NULL;
        std::map<oid_t, MatchEntity*>::iterator itr = _entities.find(id);
        if (itr == _entities.end()) {
            ent = E_NEW MatchEntity;
        } else {
            ent = itr->second;
            if (ent == NULL) {
                ent = E_NEW MatchEntity;
            } else if (ent->status == MATCH_PENDING) {
                return false;
            }
        }

        ent->id = id;
        ent->elo = elo;
        ent->time = time_s();
        ent->status = MATCH_PENDING;
        ent->size = size;

        push(ent);
        return true;
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
        que->candidates.push_back(ent->id);

        int lvl = get_rank_level(ent->elo);
        rank_map_t::iterator itr_rank = que->ranks.find(lvl);
        if (itr_rank == que->ranks.end()) {
            que->ranks.insert(std::make_pair(lvl, TeamSet()));
        }
        que->ranks[lvl].insert(ent->id);
    }


    bool MatchPool::top(MatchEntity &ent, int size_type) {
        std::map<int, MatchQueue*>::iterator itr = _queues.find(size_type);
        if (itr == _queues.end()) {
            return false;
        }
        MatchQueue *que = itr->second;
        if (que->candidates.empty()) {
            return false;
        }

        std::deque<oid_t>::iterator i_que = que->candidates.begin();
        for (;i_que != que->candidates.end();) {
            MatchEntity *curr = get_entity(*i_que);
            if (curr == NULL || curr->status != MATCH_PENDING) {
                i_que = que->candidates.erase(i_que);
            } else  {
                curr->status = MATCH_DONE;
                ent = clone_entity(curr);
                return true;
            }
        }
        return false;
    }

    /// find skill proper opponent
    bool MatchPool::get_opponent(MatchEntity *ent, MatchEntity *opp, int need_size) {
        bool ret = false;
        switch (_mode) {
        case MATCH_MOD_RAND:
            ret = get_opponent_rand(ent, opp, need_size);
            break;
        case MATCH_MOD_ELO:
            ret = get_opponent_elo(ent, opp, need_size);
            break;
        }
        return ret;
    }

    bool MatchPool::get_opponent_elo(MatchEntity *ent, MatchEntity *opp, int need_size) {
        return false;
    }

    bool MatchPool::get_opponent_rand(MatchEntity *ent, MatchEntity *opp, int need_size) {
        return get_entity_rand(need_size, opp);
    }


    bool MatchPool::get_entity_rand(int need_size, MatchEntity *dst) {
        int size = 0;
        for (int i = need_size;i >= 1; i--) {
            std::map<int, MatchQueue*>::iterator itr = _queues.find(i);
            if (itr == _queues.end()) {
                continue;
            }
            MatchQueue *que = itr->second;
            if (que == NULL || que->size_type > need_size - size) {
                continue;
            }

            // rand
            std::deque<oid_t>::iterator itr_c = que->candidates.begin();
            std::vector<oid_t> candidates;
            for (;itr_c != que->candidates.end(); ++itr_c) {
                MatchEntity *curr = get_entity(*itr_c);
                if (curr != NULL && curr->status == MATCH_PENDING) {
                    candidates.push_back(*itr_c);
                }
            }

            int size_type = que->size_type;
            if (candidates.size() == 0 ||
                candidates.size() < size_t((need_size - size) / size_type)) {
                continue;
            }
            int total = candidates.size();
            while (size + size_type <= need_size) {
                int idx = rand(0, total - 1);
                MatchEntity *curr = get_entity(candidates[idx]);
                if (curr != NULL && curr->status == MATCH_PENDING) {
                    curr->status = MATCH_DONE;
                    dst->members.push_back(clone_entity(curr));
                    size += size_type;
                }
            }
            if (size == need_size) {
                break;
            }
        }
        return size == need_size;
    }



    bool MatchPool::get_buddy(int need_size, MatchEntity *dst) {
        bool ret = false;
        switch (_mode) {
        case MATCH_MOD_RAND:
            ret = get_buddy_rand(need_size, dst);
            break;
        case MATCH_MOD_ELO:
            ret = get_buddy_elo(need_size, dst);
            break;
        }
        return ret;
    }

    bool MatchPool::get_buddy_elo(int need_size, MatchEntity *dst) {
        return NULL;
    }

    bool MatchPool::get_buddy_rand(int need_size, MatchEntity *dst) {
        return get_entity_rand(need_size, dst);
    }

    bool MatchPool::pop(int size_type, MatchRes &res) {
        std::vector<MatchEntity*> camps;
        for (int i = 0;i < _camp_size; i++) {
            MatchEntity *ent = E_NEW MatchEntity;
            ent->camp = i;
            camps.push_back(ent);
        }

        int success = pop(size_type, camps);

        // merge result
        res.type = _type;
        for (size_t i = 0;i < camps.size(); i++) {
            TeamSet members;
            camps[i]->get_members(members);
            if (!success) {
                // restore
                TeamSet::iterator itr = members.begin();
                for (;itr != members.end(); ++itr) {
                    MatchEntity *ent = get_entity(*itr);
                    if (ent) {
                        ent->status = MATCH_PENDING;
                    }
                }
            } else {
                res.teams.push_back(members);
            }
            E_DELETE camps[i];
        }

        return success;
    }

    bool MatchPool::pop(int size_type, std::vector<MatchEntity*> &camps) {
        if (camps.size() != size_t(_camp_size)) {
            return false;
        }

        // find entity for first camp
        MatchEntity *ent = camps[0];
        if (!top(*ent, size_type)) {
            return false;
        }

        // find others' camp members with size_type
        for (size_t i = 1;i < camps.size(); i++) {
            MatchEntity *opp = camps[i];
            if (!get_opponent(ent, opp, ent->size)) {
                return false;
            }
        }

        // find left members
        int size = ent->get_size();
        while (size < _team_size) {
            // find buddies
            if (!get_buddy(_team_size - size, camps[0])) {
                return false;
            }

            // find oppoents
            for (size_t i = 1;i < camps.size(); i++) {
                if (!get_opponent(camps[0], camps[i], _team_size - size)) {
                    return false;
                }
            }
            size += camps[0]->get_size();
        }
        return true;
    }

    /// tick
    bool MatchPool::tick(std::list<MatchRes> &resList) {
        bool ret = pop(resList);

        // adjust ranklevel for entities and gc.
        std::map<oid_t, MatchEntity*>::iterator itr = _entities.begin();
        for (;itr != _entities.end();) {
            MatchEntity *ent = itr->second;
            if (ent == NULL || ent->status != MATCH_PENDING) {
                if (ent != NULL) {
                    E_DELETE ent;
                }
                _entities.erase(itr++);
            } else {
                ++itr;
            }
        }

        return  ret;
    }

    /// pop matched team(s)
    bool MatchPool::pop(std::list<MatchRes> &resList) {
        for (int i = _team_size;i >= 1; i--) {
            MatchRes res;
            if (pop(i, res)) {
                resList.push_back(res);
            }
        }
        return !resList.empty();
    }

    void MatchPool::Del(const oid_t &id) {
        std::map<oid_t, MatchEntity*>::iterator itr = _entities.find(id);
        if (itr != _entities.end()) {
            MatchEntity *ent = itr->second;
            ent->status = MatchPool::MATCH_DELETED;
        }
    }

    MatchPool *MatchPool::Create(int type, int mode, int team_size, int camp_size) {
        MatchPool *pool = Get(type);
        if (pool != NULL) {
            return pool;
        }
        pool = E_NEW MatchPool(type, mode, team_size, camp_size);
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
            std::list<MatchRes> tmpList;;
            if (itr->second->tick(tmpList)) {
                resList.merge(tmpList);
            }
        }
    }

    void MatchPool::Cancel(oid_t id) {
        std::map<int, MatchPool*>::iterator itr;
        for (itr = s_pools.begin(); itr != s_pools.end(); ++itr) {
            itr->second->Del(id);
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
