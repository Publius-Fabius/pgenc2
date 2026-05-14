#include "pgenc/alloc.h"
#include "pgenc/err.h"
#include <stdio.h>
#include <stdlib.h>

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

static void test_alloc_small(void)
{
    pgc_alloc_t a;
    pgc_alloc_init(&a, 32, 2, 128, heap_alloc, heap_free, NULL);

    unsigned char *ptr1, *ptr2;

    puts("it can allocate a small memory region");
    pgc_test((ptr1 = pgc_alloc(&a, 16)) != NULL);

    puts("it can allocate another memory region");
    pgc_test((ptr2 = pgc_alloc(&a, 16)) != NULL);

    puts("the two memory regions do not overlap");
    pgc_test(ptr2 - ptr1 >= 16);

    puts("it will add pages when necessary");
    pgc_test(pgc_alloc(&a, 16) != NULL);
    pgc_test(a.num_pages == 2);
    pgc_test(pgc_alloc(&a, 16) != NULL);

    puts("it will not allocate more than num_pages");
    pgc_test(pgc_alloc(&a, 16) == NULL);

    puts("it will free the memory when allocating small objects");
    (void)pgc_alloc_free(&a);
}

static void test_alloc_large(void)
{
    pgc_alloc_t a;
    pgc_alloc_init(&a, 8, 2, 48, heap_alloc, heap_free, NULL);

    unsigned char *ptr1, *ptr2;

    puts("it can allocate a large memory region");
    pgc_test((ptr1 = pgc_alloc(&a, 32)) != NULL);

    puts("it will not allocate a region that exceeds max_alloc");
    pgc_test(pgc_alloc(&a, 64) == NULL);

    puts("it can allocate another large memory region");
    pgc_test((ptr2 = pgc_alloc(&a, 32)) != NULL);
    pgc_test(ptr2 != ptr1);

    puts("it will not allocate a large region if num_pages is exceeded");
    pgc_test(pgc_alloc(&a, 32) == NULL);

    puts("it will free all the large regions");
    (void)pgc_alloc_free(&a);
}

static void test_rewind(void)
{
    pgc_alloc_t a;
    pgc_alloc_init(&a, 16, 2, 16, heap_alloc, heap_free, NULL);

    unsigned char *ptr1;

    pgc_test(pgc_alloc(&a, 8) != NULL);
 
    pgc_alloc_snap_t s = pgc_alloc_save(&a);

    puts("it will rewind on the same page");
    pgc_test((ptr1 = pgc_alloc(&a, 8)) != NULL);

    pgc_alloc_rewind(&a, s);
    s = pgc_alloc_save(&a);
  
    pgc_test(pgc_alloc(&a, 8) == ptr1);

    puts("it will rewind from a different page");
    pgc_test(pgc_alloc(&a, 8) != NULL);
    pgc_test(a.num_pages == 2);
  
    pgc_alloc_rewind(&a, s);
    pgc_test(a.num_pages == 1);
    pgc_test(pgc_alloc(&a, 8) == ptr1);

    (void)pgc_alloc_free(&a);
}

int main(int argc, char** args)
{
    (void)argc;
    (void)args;
    (void)test_alloc_small();
    (void)test_alloc_large();
    (void)test_rewind();
}
