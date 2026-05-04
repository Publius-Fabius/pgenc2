#ifndef PGENC_STK_H
#define PGENC_STK_H

#include "pgenc/err.h"
#include <stddef.h>
#include <assert.h>

/** Byte Stack */
struct pgc_stk {
    unsigned char *base;
    unsigned char *top;
    unsigned char *fence;
};

/** Initialize the stack. */
static inline void pgc_stk_init(
    struct pgc_stk *stk, 
    void *addr, 
    const size_t len)
{
    stk->base = addr;
    stk->top = stk->base + len;
    stk->fence = stk->top;
}

/** Push len bytes to the stack, NULL on overflow, address on success. */
static inline void *pgc_stk_push(struct pgc_stk *stk, const size_t len)
{
    assert(stk->base <= stk->top);
    if ((size_t)(stk->top - stk->base) < len) return NULL;
    stk->top -= len;
    return stk->top;
}

/** Peek the stack, NULL if pointer is invalid. */
static inline void *pgc_stk_peek(const struct pgc_stk *stk, const size_t len)
{
    assert(stk->top <= stk->fence);
    if ((size_t)(stk->fence - stk->top) <= len) return NULL;
    return stk->top + len;
}

/** Pop len bytes from stack.  Accessing a memory region after it has been 
 * popped is undefined behavior.  Returns: PGC_SUFLO | PGC_OK. */
static inline int pgc_stk_pop(struct pgc_stk *stk, const size_t len)
{
    assert(stk->top <= stk->fence);
    if ((size_t)(stk->fence - stk->top) < len) return PGC_SUFLO;
    stk->top += len;
    return PGC_OK;
}

/** Get the stack's capacity. */
static inline size_t pgc_stk_capacity(const struct pgc_stk *stk)
{
    assert(stk->base <= stk->fence);
    return (size_t)(stk->fence - stk->base);
}

/** Get the stack's current size. */
static inline size_t pgc_stk_size(const struct pgc_stk *stk)
{
    assert(stk->top <= stk->fence);
    return (size_t)(stk->fence - stk->top);
}

#endif
