#ifndef PGENC_ALLOC_H
#define PGENC_ALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef struct pgc_alloc_page {
    struct pgc_alloc_page* next;
    unsigned char data[];
} pgc_alloc_page_t;

typedef struct pgc_alloc {
    pgc_alloc_page_t* head;
    size_t page_size;
    size_t max_pages;
    size_t max_alloc;
    size_t num_pages;
    size_t top;
    void* (*heap_alloc)(const size_t len, void* heap);
    void (*heap_free)(void* ptr, void* heap);
    void* heap;
} pgc_alloc_t; 

typedef struct pgc_alloc_snap {
    struct pgc_alloc_page *page;
    size_t top;
} pgc_alloc_snap_t;

static inline void pgc_alloc_init(
    pgc_alloc_t *state,
    const size_t page_size,
    const size_t max_pages,
    const size_t max_alloc,
    void *(*heap_alloc)(const size_t, void*),
    void (*heap_free)(void* ptr, void* heap),
    void* heap) 
{
    state->head = NULL;
    state->page_size = page_size;
    state->max_pages = max_pages;
    state->max_alloc = max_alloc;
    state->num_pages = 0;
    state->top = 0;
    state->heap_alloc = heap_alloc;
    state->heap_free = heap_free;
    state->heap = heap;
}

static inline void pgc_alloc_free(pgc_alloc_t *state)
{
    pgc_alloc_page_t* curr = state->head;
    while (curr) {
        pgc_alloc_page_t* next = curr->next;
        state->heap_free(curr, state->heap); 
        curr = next;
    }
    state->head = NULL;
    state->num_pages = 0;
    state->top = 0;
}

static bool pgc_alloc_expand(pgc_alloc_t *state)
{
    if (state->num_pages >= state->max_pages) return false;
    const size_t total = sizeof(pgc_alloc_page_t) + state->page_size;

    pgc_alloc_page_t* page = state->heap_alloc(total, state->heap);
    if (page == NULL) return false;
    
    page->next = state->head;
    state->top = 0;
    state->head = page;
    ++state->num_pages;
    return true;
}

static void* pgc_alloc_large(pgc_alloc_t *state, const size_t size)
{
    assert(size > state->page_size);

    if (state->num_pages >= state->max_pages ||
        size > state->max_alloc)
        return NULL;

    pgc_alloc_page_t* page = state->heap_alloc(size, state->heap);
    if (page == NULL) return NULL;
    
    page->next = state->head;
    state->top = size;
    state->head = page;
    ++state->num_pages;
    return page->data;
}

static inline void* pgc_alloc(pgc_alloc_t* state, const size_t size) 
{
    const size_t align = sizeof(size_t) - 1;
    const size_t true_size = (size + align) & ~align;

    if (true_size > state->page_size) 
        return pgc_alloc_large(state, true_size);

    if (!state->head || state->top + true_size > state->page_size) {
        if (!pgc_alloc_expand(state)) return NULL; 
    }

    void* ptr = state->head->data + state->top;
    state->top += true_size;
    return ptr;
}

static inline pgc_alloc_snap_t pgc_alloc_save(pgc_alloc_t *state)
{
    return (pgc_alloc_snap_t){ state->head, state->top };
}

static inline void pgc_alloc_rewind(
    pgc_alloc_t* state, 
    pgc_alloc_snap_t snap) 
{
    pgc_alloc_page_t* curr = state->head;
    while (curr && curr != snap.page) {
        pgc_alloc_page_t* next = curr->next;
        state->heap_free(curr, state->heap); 
        curr = next;
        --state->num_pages;
    }
    state->head = snap.page;
    state->top = snap.top;
}

#endif
