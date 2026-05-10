
#include "pgenc/par.h"
#include <stdio.h>

static void test_byte(void) 
{
    (void)puts("it can parse one constant byte");
    struct pgc_par p;
    (void)p;
}

int main(int argc, char **args)
{
    (void)argc;
    (void)args;
    (void)test_byte();
    return 0;
}
