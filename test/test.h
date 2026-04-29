#ifndef PGENC_TEST_H
#define PGENC_TEST_H

#include <stdio.h>
#include <stdlib.h>

#define test(EXPR) \
    if(!(EXPR)) { \
        fprintf(stderr, \
            "%s:%i: %s: test failed\n", \
            __FILE__, \
            __LINE__, \
            __func__); \
        abort(); \
    }

#endif
