
#include "pgenc/par.h"
#include <stdio.h>
#include <ctype.h>

static void test_byte(void) 
{
    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    (void)puts("it can parse one constant byte");
    pgc_par_t p = PGC_PAR_BYTE('a');
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

    pgc_cset_t s;
    (void)pgc_cset_from(&s, isalnum);
    pgc_par_t p = PGC_PAR_SET(&s);

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
    pgc_par_t p = PGC_PAR_UTF8(&r, 1);

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
    pgc_par_t p = PGC_PAR_AND(&cat, &dog);
    
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
    pgc_par_t p = PGC_PAR_OR(&cat, &hi);
   
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

static void *heap_alloc(const size_t len, void *st)
{
    (void)st;
    return malloc(len);
}

static void heap_free(void *ptr, void *st)
{
    (void)st;
    free(ptr);
}

static void test_str(void)
{
    pgc_alloc_t alloc;
    pgc_alloc_init(&alloc, 128, 1, 128, heap_alloc, heap_free, NULL);

    pgc_psm_t psm;
    pgc_psm_init(&psm, &alloc);

    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_par_t cat = PGC_PAR_CMP("cat", 3);
    pgc_par_t ccat = PGC_PAR_STR(&cat);
    pgc_par_t dog = PGC_PAR_CMP("dog", 3);
    pgc_par_t cdog = PGC_PAR_STR(&dog);
    pgc_par_t p = PGC_PAR_AND(&ccat, &cdog);

    puts("it can capture two strings in a row");
    psm.utag = 1245;
    pgc_test(pgc_par_runs(&p, "catdog", &stk, &psm) == 6);
    pgc_test(strcmp(psm.first->val.u.str, "cat") == 0);
    pgc_test(psm.first->val.u.str[3] == 0);
    pgc_test(psm.first->val.utag == 1245);
    pgc_test(strcmp(psm.first->nxt->val.u.str, "dog") == 0);
    pgc_test(psm.first->nxt->val.u.str[3] == 0);
    pgc_test(psm.first->nxt->nxt == NULL);

    pgc_alloc_free(&alloc);
}

static void test_num(void)
{
    pgc_alloc_t alloc;
    pgc_alloc_init(&alloc, 128, 1, 128, heap_alloc, heap_free, NULL);

    pgc_psm_t psm;
    pgc_psm_init(&psm, &alloc);

    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_par_decrec_t rec = { 10, &pgc_decimal_decoder };
     
    pgc_par_t num1 = PGC_PAR_CMP("123", 3);
    pgc_par_t cnum1 = PGC_PAR_NUM(&num1, &rec);
    pgc_par_t num2 = PGC_PAR_CMP("456", 3);
    pgc_par_t cnum2 = PGC_PAR_NUM(&num2, &rec);
    pgc_par_t p = PGC_PAR_AND(&cnum1, &cnum2);

    puts("it can capture two numbers in a row");
    psm.utag = 1245;
    pgc_test(pgc_par_runs(&p, "123456", &stk, &psm) == 6);
    pgc_test(psm.first->val.u.u64 == 123);
    pgc_test(psm.first->val.utag == 1245);
    pgc_test(psm.first->nxt->val.u.u64 == 456);
    pgc_test(psm.first->nxt->val.utag == 1245);
    pgc_test(psm.first->nxt->nxt == NULL);

    pgc_alloc_free(&alloc);
}

static void test_nest(void)
{
    pgc_alloc_t alloc;
    pgc_alloc_init(&alloc, 128, 1, 128, heap_alloc, heap_free, NULL);

    pgc_psm_t psm;
    pgc_psm_init(&psm, &alloc);

    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_par_decrec_t rec = { 10, &pgc_decimal_decoder };
     
    pgc_par_t num1 = PGC_PAR_CMP("123", 3);
    pgc_par_t cnum1 = PGC_PAR_NUM(&num1, &rec);
    pgc_par_t num2 = PGC_PAR_CMP("456", 3);
    pgc_par_t cnum2 = PGC_PAR_NUM(&num2, &rec);
    pgc_par_t and = PGC_PAR_AND(&cnum1, &cnum2);
    pgc_par_t p = PGC_PAR_NEST(&and);

    puts("it can capture a nested expression");
    psm.utag = 1245;
    pgc_test(pgc_par_runs(&p, "123456", &stk, &psm) == 6);
    pgc_test(psm.first->val.atag == PGC_AST_LST);
    pgc_ast_lst_t *l = psm.first->val.u.lst;
    pgc_test(l->val.u.u64 == 123);
    pgc_test(l->val.utag == 1245);
    pgc_test(l->nxt->val.u.u64 == 456);
    pgc_test(l->nxt->val.utag == 1245);
    pgc_test(l->nxt->nxt == NULL); 
    pgc_test(psm.first->nxt == NULL);
    
    pgc_alloc_free(&alloc);
}

static void test_utag(void)
{
    pgc_alloc_t alloc;
    pgc_alloc_init(&alloc, 128, 1, 128, heap_alloc, heap_free, NULL);

    pgc_psm_t psm;
    pgc_psm_init(&psm, &alloc);

    char *stk_bytes[128];
    pgc_stk_t stk;
    pgc_stk_init(&stk, stk_bytes, 128);

    pgc_par_decrec_t rec = { 10, &pgc_decimal_decoder };
     
    pgc_par_t num = PGC_PAR_CMP("123", 3);
    pgc_par_t cnum = PGC_PAR_NUM(&num, &rec);
    pgc_par_t tag = PGC_PAR_UTAG(321);
    pgc_par_t p = PGC_PAR_AND(&tag, &cnum);

    puts("it can tag a node");
    pgc_test(pgc_par_runs(&p, "123456", &stk, &psm) == 3);
    pgc_test(psm.first->val.utag == 321); 
    pgc_test(psm.first->nxt == NULL);

    pgc_alloc_free(&alloc);
}

int main(int argc, char **args)
{
    (void)argc;
    (void)args;
    test_byte();
    test_set(); 
    test_cmp();
    test_utf8();
    test_and();
    test_or();
    test_rep();
    test_str();
    test_num();
    test_nest();
    test_utag();
    return 0;
}
