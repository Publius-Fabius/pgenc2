
#include "pgenc/stk.h"
#include <stdio.h>
#include <stdlib.h>

static void test_push_pop(void)
{
    char bytes[5];
    struct pgc_stk stk;
    pgc_stk_init(&stk, bytes, 5);
   
    puts("it can push bytes and pop bytes");
    
    void *ptr = pgc_stk_push(&stk, 1);
    pgc_test(pgc_stk_size(&stk) == 1);
    pgc_test(ptr == bytes + 4);
    pgc_test(pgc_stk_pop(&stk, 2) == PGC_SUFLO);
    pgc_test(pgc_stk_pop(&stk, 1) == 0);
    ptr = pgc_stk_push(&stk, 4);
    pgc_test(ptr == bytes + 1);
    pgc_test(pgc_stk_size(&stk) == 4); 
    ptr = pgc_stk_push(&stk, 2);
    pgc_test(ptr == NULL);
    ptr = pgc_stk_push(&stk, 1);
    pgc_test(ptr == bytes);
    pgc_test(pgc_stk_size(&stk) == 5);
    ptr = pgc_stk_push(&stk, 1);
    pgc_test(ptr == NULL);
    pgc_test(pgc_stk_pop(&stk, 6) == PGC_SUFLO);
    pgc_test(pgc_stk_pop(&stk, 5) == 0);
    pgc_test(pgc_stk_capacity(&stk) == 5);
}

int main(int argc, char **args)
{
    (void)argc;
    (void)args;
    test_push_pop();
}
