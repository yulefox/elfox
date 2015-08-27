#ifndef __SKIPLIST_H
#define __SKIPLIST_H

namespace elf {

typedef int (*comparator)(void *obj1, void *obj2);

/* Struct to hold a inclusive/exclusive range spec by score comparison. */
typedef struct {
    double min, max;
    int minex, maxex; /* are min or max exclusive? */
} zrangespec;

typedef struct zskiplistNode {
    void *obj;
    double score;
    struct zskiplistNode *backward;
    struct zskiplistLevel {
        struct zskiplistNode *forward;
        unsigned int span;
    } level[];
} zskiplistNode;

typedef struct zskiplist {
    struct zskiplistNode *header, *tail;
    unsigned long length;
    int level;
    comparator cmp;
} zskiplist;

zskiplist *zslCreate(comparator cmp) ;
void zslFree(zskiplist *zsl);
zskiplistNode *zslInsert(zskiplist *zsl, double score, void *obj);
int zslDelete(zskiplist *zsl, double score, void *obj); 
unsigned long zslDeleteRangeByScore(zskiplist *zsl, zrangespec *range);
unsigned long zslDeleteRangeByRank(zskiplist *zsl, unsigned int start, unsigned int end);
unsigned long zslGetRank(zskiplist *zsl, double score, void *o);
zskiplistNode* zslGetElementByRank(zskiplist *zsl, unsigned long rank);
int zslIsInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range);

} // namespace elf

#endif /* !__SKIPLIST_H */
