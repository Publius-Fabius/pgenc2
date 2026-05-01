#ifndef PGENC_CODEC_H
#define PGENC_CODEC_H

#include <stdint.h>

/** Map 7bit ASCII symbols to numeric values. */
struct pgc_decode_map {
    int8_t values[128];
};

/** Map numeric values to 7bit ASCII symbols. */
struct pgc_encode_map {
    int8_t symbols[128];
};
//
// /** Decode a numeric value. */
// static inline int pgc_codec_decode(
//     unsigned char bytes,
//     const size_t nbytes,
//     const struct pgc_decode_map *table,
//     const size_t base,
//     uint64_t *result)
// {
//     assert(b != NULL && table != NULL);
//     assert(b->offset <= b->end);
//     if ((size_t)(b->end - b->offset) < n) return -1;
//     for (size_t n = 0; n < nbytes; ++n) {
//         const unsigned char symbol = *b->offset++;
//         if (symbol >= 128) return 1;
//         const int8_t value = table->values[symbol];
//         if (value == -1) return 1;
//
//     }
// }

#endif
