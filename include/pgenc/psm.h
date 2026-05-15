#ifndef PGENC_PSM_H
#define PGENC_PSM_H

#include "pgenc/ast.h"
#include "pgenc/alloc.h"

typedef struct pgc_psm {
    pgc_ast_lst_t *first;
    pgc_ast_lst_t *last;
    pgc_alloc_t *alloc;
    int32_t utag;
} pgc_psm_t;

static inline void pgc_psm_init(pgc_psm_t *psm, pgc_alloc_t *alloc)
{
    psm->first = NULL;
    psm->last = NULL;
    psm->alloc = alloc;
    psm->utag = -1;
}

static inline void pgc_psm_append(pgc_psm_t *psm, pgc_ast_lst_t *last)
{
    assert(last->nxt == NULL);
    if (psm->first == NULL) {
        assert(psm->last == NULL);
        psm->first = last;
        psm->last = last;
    } else {
        assert(psm->last->nxt == NULL);
        psm->last->nxt = last;
        psm->last = last;
    }
}

#endif
