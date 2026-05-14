
#include "pgenc/par.h"
#include <stdio.h>
#include <ctype.h>

static void test_byte(void) 
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    (void)puts("it can parse one constant byte");
    struct pgc_par p = PGC_PAR_BYTE('a');
    pgc_test(pgc_par_runs(&p, "a", &stk, NULL) == 1);

    (void)puts("it will not parse a byte when there is a bad match");
    pgc_test(pgc_par_runs(&p, "b", &stk, NULL) == PGC_NOMAT);
    
    (void)puts("it handles a buffer underflow gracefully when parsing a byte");
    pgc_test(pgc_par_runs(&p, "", &stk, NULL) == PGC_BUFLO);

    (void)puts("it handles a stack overflow gracefully when parsing a byte");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "a", &stk, NULL) == PGC_SOFLO);
}

static void test_set(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    struct pgc_cset s;
    (void)pgc_cset_from(&s, isalnum);
    struct pgc_par p = PGC_PAR_SET(&s);

    (void)puts("it can parse one byte in a set");
    pgc_test(pgc_par_runs(&p, "a", &stk, NULL) == 1);
    
    (void)puts("it can parse another byte in a set");
    pgc_test(pgc_par_runs(&p, "5", &stk, NULL) == 1);

    (void)puts("it will not parse a byte not in a set");
    pgc_test(pgc_par_runs(&p, "%", &stk, NULL) == PGC_NOMAT);

    (void)puts("it handles a buffer underflow gracefully when parsing a set");
    pgc_test(pgc_par_runs(&p, "", &stk, NULL) == PGC_BUFLO);

    (void)puts("it handles a stack overflow gracefully when parsing a set");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "a", &stk, NULL) == PGC_SOFLO);
}

static void test_cmp(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_cset_t s;
    (void)pgc_cset_from(&s, isalnum);
    pgc_par_t p = PGC_PAR_CMP("cat", 3);

    (void)puts("it can parse a constant string");
    pgc_test(pgc_par_runs(&p, "cat", &stk, NULL) == 3);

    (void)puts("it will not parse a string that does not match");
    pgc_test(pgc_par_runs(&p, "car", &stk, NULL) == PGC_NOMAT);

    (void)puts("it handles a buffer underflow when comparing a string");
    pgc_test(pgc_par_runs(&p, "ca", &stk, NULL) == PGC_BUFLO);

    (void)puts("it handles a stack overflow when comparing a string");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "cat", &stk, NULL) == PGC_SOFLO);
}

static void test_utf8(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_utf8_range_t r = { 880, 1023 };
    struct pgc_par p = PGC_PAR_UTF8(&r, 1);

    (void)puts("it can parse a UTF8 symbol");
    pgc_test(pgc_par_runs(&p, "Δ", &stk, NULL) == 2);

    (void)puts("it will not parse a UTF8 that does not match");
    pgc_test(pgc_par_runs(&p, "‰", &stk, NULL) == PGC_NOMAT);

    (void)puts("it handles a buffer underflow when parsing a UTF8 symbol");
    pgc_test(pgc_par_runs(&p, "", &stk, NULL) == PGC_BUFLO);

    (void)puts("it handles a stack overflow when parsing a UTF8");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "Δ", &stk, NULL) == PGC_SOFLO);
}

static void test_and(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);
    
    pgc_par_t cat = PGC_PAR_CMP("cat", 3);
    pgc_par_t dog = PGC_PAR_CMP("dog", 3);

    struct pgc_par p = PGC_PAR_AND(&cat, &dog);
    
    (void)puts("it can parse two parsers sequentially");
    pgc_test(pgc_par_runs(&p, "catdog", &stk, NULL) == 6);

    (void)puts("it will fail when the first parser does not match");
    pgc_test(pgc_par_runs(&p, "cardog", &stk, NULL) == PGC_NOMAT);

    (void)puts("it will fail when the second parser does not match");
    pgc_test(pgc_par_runs(&p, "catdon", &stk, NULL) == PGC_NOMAT);

    (void)puts("it will handle an underflow in the second parser");
    pgc_test(pgc_par_runs(&p, "catdo", &stk, NULL) == PGC_BUFLO);

    (void)puts("it will handle an underflow in the first parser");
    pgc_test(pgc_par_runs(&p, "ca", &stk, NULL) == PGC_BUFLO);

    (void)puts("it will handle a stack overflow in the sub-parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) * 2 - 1);
    pgc_test(pgc_par_runs(&p, "catdog", &stk, NULL) == PGC_SOFLO);

    (void)puts("it will handle a stack overflow in the main parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "catdog", &stk, NULL) == PGC_SOFLO);
}

static void test_or(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);
    
    pgc_par_t cat = PGC_PAR_CMP("cat", 3);
    pgc_par_t hi = PGC_PAR_CMP("hi", 2);

    struct pgc_par p = PGC_PAR_OR(&cat, &hi);
   
    (void)puts("it can parse the first parser in a choice");
    pgc_test(pgc_par_runs(&p, "cat", &stk, NULL) == 3);
    
    pgc_test(pgc_par_runs(&p, "hi", &stk, NULL) == 2);
    (void)puts("it can parse the second parser in a choice");

    (void)puts("it will handle an underflow in the second parser");
    pgc_test(pgc_par_runs(&p, "h", &stk, NULL) == PGC_BUFLO);
    
    (void)puts("it will handle a stack overflow in the sub-parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) * 2 - 1);
    pgc_test(pgc_par_runs(&p, "catdog", &stk, NULL) == PGC_SOFLO);
    
    (void)puts("it will handle a stack overflow in the main parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "catdog", &stk, NULL) == PGC_SOFLO);
}

static void test_rep(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);
  
    pgc_cset_t set;
    pgc_cset_from(&set, isalnum);
    pgc_par_t sub = PGC_PAR_SET(&set);
    pgc_par_t p = PGC_PAR_REP(&sub, 2, 3);
  
    (void)puts("it can repeat a parser a minimum number of times");
    pgc_test(pgc_par_runs(&p, "x1%", &stk, NULL) == 2);
   
    (void)puts("it can repeat a parser a maximum number of times");
    pgc_test(pgc_par_runs(&p, "x1abc%", &stk, NULL) == 3);

    (void)puts("it will not parse less than the minimum number of repetions");
    pgc_test(pgc_par_runs(&p, "x%", &stk, NULL) == PGC_NOMAT);
  
    (void)puts("it will buffer underflow if less than minimum repetitions");
    pgc_test(pgc_par_runs(&p, "x", &stk, NULL) == PGC_BUFLO);
  
    (void)puts("it will handle a stack overflow in the sub-parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) * 2 - 1);
    pgc_test(pgc_par_runs(&p, "x1ABC", &stk, NULL) == PGC_SOFLO);
  
    (void)puts("it will handle a stack overflow in the main parser");
    pgc_stk_init(&stk, stk_bytes, sizeof(void*) - 1);
    pgc_test(pgc_par_runs(&p, "x1abc", &stk, NULL) == PGC_SOFLO);
}

static void test_str(void)
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_cset_t set;
    pgc_cset_from(&set, isalnum);
    pgc_par_t sub = PGC_PAR_SET(&set);
    pgc_par_t p = PGC_PAR_REP(&sub, 2, 3);

}

int main(int argc, char **args)
{
    (void)argc;
    (void)args;
    (void)test_byte();
    (void)test_set(); 
    (void)test_cmp();
    (void)test_utf8();
    (void)test_and();
    (void)test_or();
    (void)test_rep();
    (void)test_str();
    return 0;
}
