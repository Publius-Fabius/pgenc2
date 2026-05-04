#ifndef PGENC_ERR_H
#define PGENC_ERR_H

#include <stdio.h>
#include <stdlib.h>

enum pgc_err {
    PGC_OK                      = 0,            // all ok 
    PGC_ERRNO                   = -1,           // errno error
    PGC_ISEEK                   = -10,          // invalid seek 
    PGC_BOFLO                   = -11,          // buffer overflow
    PGC_BUFLO                   = -12,          // buffer underflow
    PGC_SOFLO                   = -13,          // stack overflow
    PGC_SUFLO                   = -14,          // stack underflow
    PGC_NOMAT                   = -15,          // no match found
    PGC_IUTF8                   = -16,          // invalid utf8 
    PGC_UNRCH                   = -17           // unreachable state
};

static inline void pgc_panic_(
    const char *file,
    const int line,
    const char *func,
    const char *msg)
{
    (void)fflush(stdout);
    (void)fprintf(
        stderr,
        "\n[PANIC] %s:%i: %s: %s\n",
        file,
        line,
        func,
        msg);
    (void)fflush(stderr);
    (void)abort();
}

#define pgc_panic(MSG) pgc_panic_(__FILE__, __LINE__, __func__, (MSG))

#define pgc_test(EXPR)                  \
    if (!(EXPR)) {                      \
        pgc_panic("test failed");       \
    }

#define pgc_try(EXPR)                   \
    do {                                \
        const int _err000 = (EXPR);     \
        if (_err000 < 0) {              \
            return _err000;             \
        }                               \
    } while (0)

#define pgc_always(EXPR)                \
    do {                                \
        const int _err000 = (EXPR);     \
        assert(_err000 >= 0);           \
        (void)_err000;                  \
    } while (0)

#endif
