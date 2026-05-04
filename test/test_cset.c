
#include "pgenc/err.h"
#include "pgenc/cset.h"
#include <stdio.h>
#include <ctype.h>

static void test_set_in(void)
{
    puts("it can set an element from 0-255");
    for (unsigned int x = 0; x < 256; ++x) {
        struct pgc_cset set; 
        pgc_cset_zero(&set);
        pgc_cset_set(&set, (uint8_t)x);
        pgc_test(pgc_cset_in(&set, (uint8_t)x));
    }
}

static void test_set_unset(void) 
{
    puts("it can unset an element from 0-255");
    for (unsigned int c = 0; c < 256; ++c) {
        struct pgc_cset set;
        pgc_cset_zero(&set);
        pgc_cset_set(&set, (uint8_t)c);
        pgc_test(pgc_cset_in(&set, (uint8_t)c));
        pgc_cset_unset(&set, (uint8_t)c);
        pgc_test(!pgc_cset_in(&set, (uint8_t)c));
        for (unsigned int d = 0; d < 256; ++d) {
            pgc_test(!pgc_cset_in(&set, (uint8_t)d));
        }
    }
}

static void test_from(void)
{
    puts("it builds correctly from a ctype.h function");
    struct pgc_cset set;
    pgc_cset_from(&set, isdigit);
    for (unsigned int c = 0; c < 256; ++c) {
        if ('0' <= c && c <= '9') {
            pgc_test(pgc_cset_in(&set, (uint8_t)c));
        } else {
            pgc_test(!pgc_cset_in(&set, (uint8_t)c));
        }
    }
}

static void test_union(void)
{
    puts("it can compute the union of two ctype.h functions");
    struct pgc_cset setA;
    struct pgc_cset setB;
    struct pgc_cset setC;
    struct pgc_cset setD;
    pgc_cset_from(&setA, isdigit);
    pgc_cset_from(&setB, isalpha);
    pgc_cset_union(&setC, &setA, &setB);
    pgc_cset_from(&setD, isalnum);
    for (int x = 0; x < 4; ++x) {
        pgc_test(setC.words[x] == setD.words[x]);
    }
}

static void test_diff(void)
{
    puts("it can compute the difference of two ctype.h functions");
    struct pgc_cset setA;
    struct pgc_cset setB;
    struct pgc_cset setC;
    struct pgc_cset setD;
    pgc_cset_from(&setA, isalnum);
    pgc_cset_from(&setB, isalpha);
    pgc_cset_diff(&setC, &setA, &setB);
    pgc_cset_from(&setD, isdigit);
    for (int x = 0; x < 4; ++x) {
        pgc_test(setC.words[x] == setD.words[x]);
    }
}

static void test_isect(void)
{
    puts("it can compute the intersection of two ctype.h functions");
    struct pgc_cset setA;
    struct pgc_cset setB;
    struct pgc_cset setC;
    struct pgc_cset setD;
    pgc_cset_from(&setA, isalnum);
    pgc_cset_from(&setB, isdigit);
    pgc_cset_isect(&setC, &setA, &setB);
    pgc_cset_from(&setD, isdigit);
    for (int x = 0; x < 4; ++x) {
        pgc_test(setC.words[x] == setD.words[x]);
    }
}

static void test_not(void)
{
    puts("it can compute the complement of a ctype.h function");
    struct pgc_cset setA;
    struct pgc_cset setB;
    pgc_cset_from(&setA, isdigit);
    pgc_cset_not(&setB, &setA);
    for (unsigned int x = 0; x < 256; ++x) {
        if('0' <= x && x <= '9') {
            pgc_test(!pgc_cset_in(&setB, (uint8_t)x));
        } else {
            pgc_test(pgc_cset_in(&setB, (uint8_t)x));
        }
    }
}

int main(int argc, char **args) 
{
    (void)argc;
    (void)args;
    puts("testing cset");
    test_set_in();
    test_set_unset();
    test_from();
    test_union();
    test_diff();
    test_isect();
    test_not();
    puts("all cset tests passed");
}
