#ifndef PGENC_CODEC_H
#define PGENC_CODEC_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

/** Map 7bit ASCII symbols to numeric values. */
struct pgc_decoder {
    int8_t values[128];
};

/** Map numeric values to 7bit ASCII symbols. */
struct pgc_encoder {
    int8_t symbols[128];
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

/** Encode numeric value, -1 on buf overflow, */
static inline int pgc_encode(
    const uint64_t value,
    const size_t base,
    const struct pgc_encoder *table,
    void *buf,
    const size_t len) 
{
    char tmp[72];
    assert(base <= 128);
    uint64_t reduced = value;
    for(int n = 0; n < 72; ++n) {
        if (len <= (size_t)n) return -1;
        const int index = 71 - n;
        tmp[index] = table->symbols[reduced % base];
        reduced /= base;
        if (reduced == 0) {
            const int tmp_len = n + 1;
            memcpy(buf, tmp + index, (size_t)tmp_len);
            return tmp_len;
        }
    }
    assert(0);
    return -1;
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
