#ifndef PGENC_CODEC_H
#define PGENC_CODEC_H

#include "pgenc/err.h"
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

/** Map 7bit ASCII symbols to numeric values. */
struct pgc_decoder {
    alignas(64) int8_t values[128];
};

/** Map numeric values to 7bit ASCII symbols. */
struct pgc_encoder {
    alignas(64) int8_t symbols[128];
};

/** Decode a numeric value. */
static inline void pgc_decode(
    const void* addr,
    const size_t nbytes,
    const size_t base,
    const struct pgc_decoder *table,
    uint64_t *result)
{
    const uint8_t *bytes = addr;
    *result = 0ULL;
    for (size_t n = 0; n < nbytes; ++n) {
        *result = *result * base + (uint64_t)table->values[bytes[n]]; 
    }
}

/** Encode numeric value.  Returns: chars writen | PGC_BOFLO | PGC_UNRCH. */
static inline int pgc_encode(
    const uint64_t value,
    const size_t base,
    const struct pgc_encoder *table,
    void* buf,
    const size_t len) 
{
    char tmp[72];
    assert(base <= 128);
    uint64_t reduced = value;
    for (int n = 0; n < 72; ++n) {
        if (len <= (size_t)n) return PGC_BOFLO;
        const int index = 71 - n;
        tmp[index] = table->symbols[reduced % base];
        reduced /= base;
        if (reduced == 0) {
            const int tmp_len = n + 1;
            (void)memcpy(buf, tmp + index, (size_t)tmp_len);
            return tmp_len;
        }
    }
    pgc_panic("unreachable state"); 
    return PGC_UNRCH;
}

static const struct pgc_decoder pgc_decimal_decoder = { {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0-15 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16-31 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32-47 
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, // 48-63 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64-79
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80-95
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96-111
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 112-127
} };

static const struct pgc_decoder pgc_hex_decoder = { {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0-15 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16-31 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32-47 
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, // 48-63 
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64-79
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80-95
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96-111
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 112-127
} };

static const struct pgc_encoder pgc_decimal_encoder = { {
    '0','1','2','3','4','5','6','7','8','9',-1, -1, -1, -1, -1, -1, // 0-15 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16-31 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32-47 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 48-63 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64-79
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80-95
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96-111
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 112-127
} };

static const struct pgc_encoder pgc_hex_encoder = { {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',// 0-15 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16-31 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32-47 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 48-63 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64-79
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80-95
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96-111
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 112-127
} };

#endif
