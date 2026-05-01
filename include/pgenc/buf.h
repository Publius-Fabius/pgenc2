#ifndef PGC_BUF_H
#define PGC_BUF_H

#include "pgenc/cset.h"
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

/** [ base, .., offset, .., end, .., fence ] */
struct pgc_buf {
    unsigned char *base;
    unsigned char *offset;
    unsigned char *end;
    unsigned char *fence;
    uint64_t absolute;
};

static inline void pgc_buf_init(
    struct pgc_buf *b, 
    void *addr,
    const size_t len)
{
    assert(b);
    b->base = addr;
    b->offset = b->base;
    b->end = b->base;
    b->fence = b->base + len;
    b->absolute = 0;
}

static inline size_t pgc_buf_capacity(const struct pgc_buf *b) 
{
    assert(b != NULL);
    assert(b->base <= b->fence);
    return (size_t)(b->fence - b->base);
}

static inline size_t pgc_buf_available(const struct pgc_buf *b)
{
    assert(b != NULL);
    assert(b->base <= b->offset);
    assert(b->end <= b->fence);
    const size_t tail = (size_t)(b->fence - b->end);
    const size_t trash = (size_t)(b->offset - b->base);
    return tail + trash;
}

/** Reserve 'n' bytes at end of the buffer. */
static inline int pgc_buf_reserve(struct pgc_buf *b, const size_t n) 
{
    assert(b != NULL);
    assert(b->base <= b->offset);
    assert(b->offset <= b->end);
    assert(b->end <= b->fence);

    const size_t tail = (size_t)(b->fence - b->end);
    if (n <= tail) return 0;
    
    const size_t trash = (size_t)(b->offset - b->base);
    if (tail + trash < n) return -1;
    
    const size_t active = (size_t)(b->end - b->offset);
    memmove(b->base, b->offset, active);
    
    b->absolute += trash;
    b->offset = b->base;
    b->end = b->base + active;
    return 0;   
}

static inline int pgc_buf_put_char(struct pgc_buf *b, unsigned char c)
{
    if (pgc_buf_reserve(b, 1) != 0) return -1;
    *b->end++ = c;
    return 0;
}

static inline int pgc_buf_peek_char(const struct pgc_buf *b) 
{
    assert(b != NULL);
    if (b->offset >= b->end) return -1;
    return (int)*b->offset;
}

static inline int pgc_buf_get_char(struct pgc_buf *b)
{
    assert(b != NULL);
    if (b->offset >= b->end) return -1;
    return (int)*b->offset++;
}

/** Get the buffer's current position */
static inline uint64_t pgc_buf_tell(const struct pgc_buf *b) 
{
    assert(b != NULL);
    assert(b->base <= b->offset);
    return b->absolute + (uint64_t)(b->offset - b->base);
}

/** Seek to buffer position. */
static inline int pgc_buf_seek(struct pgc_buf *b, const uint64_t p) 
{
    assert(b != NULL);
    if (b->absolute > p) return -1;
    unsigned char *new_offset = b->base + (size_t)(p - b->absolute);
    if (new_offset > b->end) return -1;
    b->offset = new_offset;
    return 0;
}

/** Write n bytes of m to the buffer, returns 0 on success, -1 on OOB. */
static inline int pgc_buf_put(struct pgc_buf *b, const void *m, const size_t n)
{
    assert(m != NULL);
    if (pgc_buf_reserve(b, n) != 0) return -1;
    memcpy(b->end, m, n);
    b->end += n;
    return 0;
}

/** Get up to n bytes, store them in m, returns total bytes copied. */
static inline size_t pgc_buf_get(struct pgc_buf *b, void *m, const size_t n)
{
    assert(b != NULL && m != NULL);
    assert(b->offset <= b->end);
    const size_t active = (size_t)(b->end - b->offset);
    const size_t min = n <= active ? n : active; 
    memcpy(m, b->offset, min);
    b->offset += min;
    return min;
}

/** Compare n bytes of m, return -1 on OOB, 1 on success, 0 on mismatch. */
static inline int pgc_buf_test_str(
    const struct pgc_buf *b,
    const void *m, 
    const size_t n)
{
    assert(b != NULL && m != NULL);
    assert(b->offset <= b->end);
    if ((size_t)(b->end - b->offset) < n) return -1;
    if (memcmp(b->offset, m, n) == 0) return 1;
    return 0;
}

/** Match n bytes of m, same as test_str but consumes bytes. */
static inline int pgc_buf_match_str(
    struct pgc_buf *b, 
    const void *m, 
    const size_t n)
{
    const int res = pgc_buf_test_str(b, m, n);
    if (res == 1) b->offset += n;
    return res;
}

/** Returns 1 if next char is in set, -1 if OOB, 0 if not in set. */
static inline int pgc_buf_test_char(
    const struct pgc_buf *b, 
    const struct pgc_cset *s) 
{
    assert(b != NULL && s != NULL);
    if (b->offset >= b->end) return -1;
    if (pgc_cset_in(s, *b->offset)) return 1;
    return 0;
}

/** Same as pgc_buf_test_char, but consumes the character. */ 
static inline int pgc_buf_match_char(
    struct pgc_buf *b, 
    const struct pgc_cset *s) 
{
    const int res = pgc_buf_test_char(b, s);
    if (res == 1) ++b->offset;
    return res;
}

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const uint8_t utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

static inline uint32_t decode_utf8(
    uint32_t* state, 
    uint32_t* codep, 
    uint32_t byte) 
{
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3fu) | (*codep << 6) :
        (0xff >> type) & (byte);

    *state = utf8d[256 + *state + type];
    return *state;
}

/* End decode_utf8 section.  Thanks Bjoern! */

/** Peek UTF8 value, returns bytes read, -1 on OOB, 0 on bad UTF8. */
static inline int pgc_buf_peek_utf8(const struct pgc_buf *b, uint32_t *v)
{
    assert(b != NULL && v != NULL);
    uint32_t utf_state = UTF8_ACCEPT;
    for (int i = 0; i < 4; ++i) {
        if (b->offset + i >= b->end) return -1;
        decode_utf8(&utf_state, v, b->offset[i]);
        if (utf_state == UTF8_ACCEPT) {
            return i + 1;
        } else if (utf_state == UTF8_REJECT) {
            return 0;    
        }
    }
    assert(0);
    return 0;
}

/** Get UTF8 value, same as pgc_buf_peeku but consumes the content. */
static inline int pgc_buf_get_utf8(struct pgc_buf *b, uint32_t *v) 
{
    const int res = pgc_buf_peek_utf8(b, v);
    if (res > 0) b->offset += res;
    return res;
}

/** Check UTF8 value, bytes checked on success, -1 on OOB, 0 on mismatch. */
static inline int pgc_buf_test_utf8(
    struct pgc_buf *b,
    bool (*pred)(const uint32_t, void*),
    void *state)
{
    uint32_t v;
    const int res = pgc_buf_peek_utf8(b, &v);
    if (res > 0) {
        if (pred(v, state)) return res;
        else return 0;
    } 
    return res;
}

/** Same as pgc_buf_test_utf8, but consumes the bytes. */
static inline int pgc_buf_match_utf8(
    struct pgc_buf *b,
    bool (*pred)(const uint32_t, void*),
    void *state)
{
    const int res = pgc_buf_test_utf8(b, pred, state);
    if (res > 0) b->offset += res;
    return res;
}

/** Request nb bytes at the end of the buffer */
static inline void* pgc_buf_request(struct pgc_buf *b, const size_t nb)
{
    if (pgc_buf_reserve(b, nb) == -1) return NULL;
    return b->end;
}

/** Advance the end of buffer nb nbytes. */
static inline void pgc_buf_advance(struct pgc_buf *b, const size_t nb)
{
    assert(b != NULL);
    b->end += nb;
    assert(b->end <= b->fence);
}

/** Claim nb bytes after the offset */
static inline void* pgc_buf_claim(const struct pgc_buf *b, const size_t nb)
{
    assert(b != NULL);
    assert(b->offset <= b->end);
    if ((size_t)(b->end - b->offset) < nb) return NULL;
    return b->offset;
}

/** Consume nb bytes after the offset. */
static inline void pgc_buf_consume(struct pgc_buf *b, const size_t nb)
{
    assert(b != NULL);
    b->offset += nb;
    assert(b->offset <= b->end);
}

#endif 
