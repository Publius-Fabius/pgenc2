#ifndef PGENC_CSET_H
#define PGENC_CSET_H

#include <stdalign.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*pgc_cset_pred_t)(int);

/** Bit Map 256 */
struct pgc_cset {
    alignas(32) uint64_t words[4];    
};

/** Zero out the set. */
static inline void pgc_cset_zero(struct pgc_cset *set)
{
    for (int x = 0; x < 4; ++x) {
        set->words[x] = 0ULL;
    }
}

/** Is elem in set? */
static inline bool pgc_cset_in(const struct pgc_cset *set, const uint8_t elem) 
{
    return (set->words[elem >> 6] & (1ULL << (elem & 63))) != 0;
}

/** Set bit. */
static inline void pgc_cset_set(struct pgc_cset *set, const uint8_t elem)
{
    set->words[elem >> 6] |= (1ULL << (elem & 63)); 
}

/** Unset bit. */
static inline void pgc_cset_unset(struct pgc_cset *set, const uint8_t elem)
{
    set->words[elem >> 6] &= ~(1ULL << (elem & 63));
}

/** Construct set from "ctype.h" function. */
static inline void pgc_cset_from(struct pgc_cset *dest, pgc_cset_pred_t pred)
{
    pgc_cset_zero(dest);
    for (unsigned int c = 0; c < 256; ++c) { 
        if (pred((int)c)) {
            pgc_cset_set(dest, (uint8_t)c);
        }
    }
}

/** Compute set intersection. */
static inline void pgc_cset_isect(
    struct pgc_cset *dest, 
    const struct pgc_cset *fst, 
    const struct pgc_cset *snd)
{
    for (int i = 0; i < 4; ++i) {
        dest->words[i] = fst->words[i] & snd->words[i];
    }
}

/** Compute set union. */
static inline void pgc_cset_union(
    struct pgc_cset *dest,
    const struct pgc_cset *fst, 
    const struct pgc_cset *snd)
{
    for (int i = 0; i < 4; ++i) {
        dest->words[i] = fst->words[i] | snd->words[i];
    }
}

/** Compute set difference. */
static inline void pgc_cset_diff(
    struct pgc_cset *dest,
    const struct pgc_cset *fst, 
    const struct pgc_cset *snd)
{
    for (int i = 0; i < 4; ++i) {
        dest->words[i] = fst->words[i] & ~snd->words[i];
    }
}

/** Compute set complement. */
static inline void pgc_cset_not(
    struct pgc_cset *dest, 
    const struct pgc_cset *src)
{
    for (int i = 0; i < 4; ++i) {
        dest->words[i] = ~src->words[i];
    }
}

#endif
