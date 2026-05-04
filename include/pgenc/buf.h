#ifndef PGC_BUF_H
#define PGC_BUF_H

#include "pgenc/err.h"
#include "pgenc/cset.h"
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

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

/** Returns buffer's maximum capacity. */
static inline size_t pgc_buf_capacity(const struct pgc_buf *b) 
{
    assert(b->base <= b->fence);
    return (size_t)(b->fence - b->base);
}

/** Get the number of available bytes (capacity - size). */
static inline size_t pgc_buf_available(const struct pgc_buf *b)
{
    assert(b->base <= b->offset);
    assert(b->end <= b->fence);
    const size_t tail = (size_t)(b->fence - b->end);
    const size_t trash = (size_t)(b->offset - b->base);
    return tail + trash;
}

/** Reserve 'n' bytes at end of the buffer.  Returns: PGC_OK | PGC_BOFLO. */
static inline int pgc_buf_reserve(struct pgc_buf *b, const size_t n) 
{
    assert(b->base <= b->offset);
    assert(b->offset <= b->end);
    assert(b->end <= b->fence);

    const size_t tail = (size_t)(b->fence - b->end);
    if (n <= tail) return PGC_OK;
    
    const size_t trash = (size_t)(b->offset - b->base);
    if (tail + trash < n) return PGC_BOFLO;
    
    const size_t active = (size_t)(b->end - b->offset);
    (void)memmove(b->base, b->offset, active);
    
    b->absolute += trash;
    b->offset = b->base;
    b->end = b->base + active;
    return PGC_OK;
}

/** Put char.  Returns: PGC_OK | PGC_BOFLO. */
static inline int pgc_buf_put_char(struct pgc_buf *b, const unsigned char c)
{
    pgc_try(pgc_buf_reserve(b, 1));
    *b->end++ = c;
    return PGC_OK;
}

/** Get the next character. Returns: Char | PGC_BUFLO. */
static inline int pgc_buf_peek_char(const struct pgc_buf *b) 
{
    return b->offset < b->end ? (int)*b->offset : PGC_BUFLO;
}

/** Get the next character.  Returns: Char | PGC_BUFLO. */
static inline int pgc_buf_get_char(struct pgc_buf *b)
{
    return b->offset < b->end ? (int)*b->offset++ : PGC_BUFLO;
}

/** Test char. Returns: PGC_OK | PGC_NOMAT | PGC_BUFLO. */
static inline int pgc_buf_test_char(const struct pgc_buf *b, const int c)
{
    return b->offset < b->end ?
        ((int)*b->offset == c ? PGC_OK : PGC_NOMAT) : PGC_BUFLO;
}

/** If test consume char.  Returns: PGC_OK | PGC_BUFLO | PGC_NOMAT. */
static inline int pgc_buf_match_char(struct pgc_buf *b, const int c)
{
    const int res = pgc_buf_test_char(b, c);
    if (res == PGC_OK) ++b->offset;
    return res;
}

/** Get the buffer's current position */
static inline uint64_t pgc_buf_tell(const struct pgc_buf *b) 
{
    assert(b->base <= b->offset);
    return b->absolute + (uint64_t)(b->offset - b->base);
}

/** Seek to buffer position.  Attempting to seek past a value that was not
 * returned by pgc_buf_tell is undefined behavior.  
 * Returns: PGC_OK, PGC_ISEEK. */
static inline int pgc_buf_seek(struct pgc_buf *b, const uint64_t p) 
{
    if (b->absolute > p) return PGC_ISEEK;
    b->offset = b->base + (size_t)(p - b->absolute);
    assert(b->offset <= b->end);
    return PGC_OK;
}

/** Write n bytes of m.  Returns: PGC_OK, PGC_BOFLO. */
static inline int pgc_buf_put(struct pgc_buf *b, const void *m, const size_t n)
{
    pgc_try(pgc_buf_reserve(b, n));
    (void)memcpy(b->end, m, n);
    b->end += n;
    return PGC_OK;
}

/** Get up to n bytes, store them in m.  Returns total bytes copied. */
static inline size_t pgc_buf_get(struct pgc_buf *b, void *m, const size_t n)
{
    assert(b->offset <= b->end);
    const size_t active = (size_t)(b->end - b->offset);
    const size_t min = n <= active ? n : active; 
    (void)memcpy(m, b->offset, min);
    b->offset += min;
    return min;
}

/** Compare n bytes of m.  Returns PGC_OK | PGC_NOMAT | PGC_BUFLO. */
static inline int pgc_buf_test_str(
    const struct pgc_buf *b,
    const void *m, 
    const size_t n)
{
    assert(b->offset <= b->end);
    return n <= (size_t)(b->end - b->offset) ?
        (memcmp(b->offset, m, n) == 0 ? PGC_OK : PGC_NOMAT) : PGC_BUFLO;
}

/** Match n bytes of m, same as test_str but consumes bytes. */
static inline int pgc_buf_match_str(
    struct pgc_buf *b, 
    const void *m, 
    const size_t n)
{
    const int res = pgc_buf_test_str(b, m, n);
    if (res == PGC_OK) b->offset += n;
    return res;
}

/** Returns 1 if next char is in set, -1 if OOB, 0 if not in set. */
static inline int pgc_buf_test_set(
    const struct pgc_buf *b, 
    const struct pgc_cset *s) 
{
    return b->offset < b->end ?
        (pgc_cset_in(s, *b->offset) ? PGC_OK : PGC_NOMAT) : PGC_BUFLO;
}

/** Same as pgc_buf_test_char, but consumes the character. */ 
static inline int pgc_buf_match_set(
    struct pgc_buf *b, 
    const struct pgc_cset *s) 
{
    const int res = pgc_buf_test_set(b, s);
    if (res == PGC_OK) ++b->offset;
    return res;
}

/* 
 * The decode_utf8 section is copy-pasted from Bjoern's website.
 * It has been released under the MIT License.
 */

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

static inline void decode_utf8(
    uint32_t* state, 
    uint32_t* codep, 
    uint32_t byte) 
{
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3fu) | (*codep << 6) :
        (0xff >> type) & (byte);

    *state = utf8d[256 + *state + type];
}

/* 
 * End decode_utf8 section. 
 */

/** Peek UTF8.  Returns: symbol_length | PGC_IUTF8 | PGC_BUFLO | PGC_UNRCH. */
static inline int pgc_buf_peek_utf8(const struct pgc_buf *b, uint32_t *v)
{
    uint32_t utf_state = UTF8_ACCEPT;
    for (int i = 0; i < 4; ++i) {
        if (b->offset + i >= b->end) return PGC_BUFLO;
        (void)decode_utf8(&utf_state, v, b->offset[i]);
        if (utf_state == UTF8_ACCEPT) return i + 1;
        if (utf_state == UTF8_REJECT) return PGC_IUTF8;
    }
    pgc_panic("unreachable state"); 
    return PGC_UNRCH;
}

/** Get UTF8.  Returns: symbol_length | PGC_IUTF8 | PGC_BUFLO | PGC_UNRCH. */
static inline int pgc_buf_get_utf8(struct pgc_buf *b, uint32_t *v) 
{
    const int res = pgc_buf_peek_utf8(b, v);
    if (res > 0) b->offset += res;
    return res;
}

/** Range of UTF8 values. */
struct pgc_utf8_range {
    uint32_t start;
    uint32_t stop;
};

/** 
 * Test UTF8.
 * Returns: symbol_length | PGC_IUTF8 | PGC_NOMAT | PGC_BUFLO | PGC_UNRCH. 
 */
static inline int pgc_buf_test_utf8(
    const struct pgc_buf *b,
    const struct pgc_utf8_range *rs, 
    const size_t nrs)
{
    uint32_t code_p;
    const int res = pgc_buf_peek_utf8(b, &code_p);
    if (res <= 0) return res;
    for (size_t i = 0; i < nrs; ++i) {
        if ((code_p - rs[i].start) <= (rs[i].stop - rs[i].start)) 
            return res;    
    }
    return PGC_NOMAT;
}
/** 
 * Test-Consume UTF8.
 * Returns: symbol_length | PGC_IUTF8 | PGC_NOMAT | PGC_BUFLO | PGC_UNRCH. 
 */
static inline int pgc_buf_match_utf8(
    struct pgc_buf *b,
    const struct pgc_utf8_range *rs,
    const size_t nrs)
{
    const int res = pgc_buf_test_utf8(b, rs, nrs);
    if (res > 0) b->offset += res;
    return res;
}

/** Request len bytes at the end of the buffer, NULL if not available. */
static inline void* pgc_buf_request(struct pgc_buf *b, const size_t len)
{
    if (pgc_buf_reserve(b, len) == -1) return NULL;
    return b->end;
}

/** Advance the end of buffer len bytes, advancing past request len is UB. */
static inline void pgc_buf_advance(struct pgc_buf *b, const size_t len)
{
    b->end += len;
    assert(b->end <= b->fence);
}

/** Claim len bytes after the offset, NULL if not available. */
static inline void* pgc_buf_claim(const struct pgc_buf *b, const size_t len)
{
    assert(b->offset <= b->end);
    if ((size_t)(b->end - b->offset) < len) return NULL;
    return b->offset;
}

/** Consume len bytes after the offset, consuming past claim len is UB. */
static inline void pgc_buf_consume(struct pgc_buf *b, const size_t len)
{
    b->offset += len;
    assert(b->offset <= b->end);
}

#endif 
