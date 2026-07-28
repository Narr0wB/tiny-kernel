
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <tiny/types.h>
#include <tiny/compiler.h>
#include <tiny/list.h>

#define HASH_MAGIC 0x9E2779B97F4A7C15ULL

struct hlist_node {
    struct hlist_node *next; 
    struct hlist_node **pprev;
};

struct hlist_head {
    struct hlist_node *first;
};

#define HLIST_HEAD_INIT \
    { .first = NULL }

#define DEFINE_HASHTABLE(name, bits) \
    struct hlist_head name[1U << (bits)] = \
        { [0 ... ((1U << (bits)) - 1)] = HLIST_HEAD_INIT }

#define HASH_SIZE(x) (ARRAY_SIZE(x))
#define HASH_BITS(x) (ilog2(ARRAY_SIZE(x)))

static __force_inline uint32_t hash(uint64_t key, uint8_t bits)
{
    return (key * HASH_MAGIC) / (1ULL << bits);
}

static __force_inline void hlist_add_head(struct hlist_head *bucket, struct hlist_node *node)
{
    node->next = bucket->first;
    node->pprev = &bucket->first;
    bucket->first = node;
    if (node->next)
        node->next->pprev = &node->next;
}

static __force_inline void hlist_del(struct hlist_node *node)
{
    struct hlist_node *prev = container_of(node->pprev, struct hlist_node, pprev);
    struct hlist_node *next = node->next;

    prev->next = next;
    next->pprev = &prev->next;
}

#define hlist_entry_safe(ptr, type, member) \
    ({ typeof(ptr) __ptr = ptr; (ptr) ? container_of(ptr, type, member) : NULL })

#define hlist_for_each_entry(pos, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); \
         pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_possible(name, obj, member, key) \
    hlist_for_each_entry(obj, &name[hash(key, HASH_BITS(name))], member)

#endif // HASHTABLE_H