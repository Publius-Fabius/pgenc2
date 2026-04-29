#ifndef PGC_BUF_H
#define PGC_BUF_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

struct pgc_buf {
    unsigned char *base;
    unsigned char *begin;
    unsigned char *offset;
    unsigned char *end;
    uint64_t absolute;
};

/** Get the buffer's current position */
static inline uint64_t pgc_buf_tell(const struct pgc_buf *b) {
    assert(b != NULL);
    uintptr_t offset = (uintptr_t)b->offset;
    uintptr_t base = (uintptr_t)b->base;
    assert(base <= offset);
    return b->absolute + (uint64_t)(offset - base);
}

/** Seek to buffer position. */
static inline int pgc_buf_seek(struct pgc_buf *b, const uint64_t p) {
    assert(b != NULL);
    if(b->absolute > p)
        return -1;
    unsigned char *new_offset = b->base + (uintptr_t)(p - b->absolute);
    assert(b->base <= new_offset && new_offset <= b->end);
    b->offset = new_offset;
    return 0;
}

/** Reserve 'n' bytes at end of the buffer. */
static inline int pgc_buf_reserve(struct pgc_buf *b, const size_t n) {
    assert(b != NULL);
    const uintptr_t end = (uintptr_t)b->end;
    const uintptr_t offset = (uintptr_t)b->offset;
    assert(offset <= end);
    const uintptr_t tail = end - offset;
    if(n <= tail)
        return 0;
    const uintptr_t base = (uintptr_t)b->base;
    const uintptr_t begin = (uintptr_t)b->begin;
    assert(base <= begin);
    const uintptr_t trash = begin - base;
    if(tail + trash < n)
        return -1;
    assert(begin < offset);
    memmove(b->base, b->begin, (size_t)(offset - begin));
    b->absolute += trash;
    b->begin = b->base;
    b->offset -= trash;
    return 0;   
}
#endif 
