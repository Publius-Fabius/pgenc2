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

static inline void pgc_psm_concat_front(
    pgc_psm_t *psm, 
    pgc_ast_lst_t *first, 
    pgc_ast_lst_t *last)
{
    assert(last->nxt == NULL);
    if (psm->first == NULL) {
        assert(psm->last == NULL);
        psm->first = first;
        psm->last = last;
    } else {
        last->nxt = psm->first;
        psm->first = first;
    }
}

enum pgc_psm_op {
    PGC_PSM_READ_U64,
    PGC_PSM_READ_STR,
    PGC_PSM_READ_UTF,

    PGC_PSM_SAVE,
    PGC_PSM_REWIND,

    /* Frame Types ...
     *
     * save cursor                  - for reading a term | fast rollback
     * save head + tail             - for structuring AST 
     * save all                     - full rollback
     */

    PGC_PSM_PUSH_FRAME,
    PGC_PSM_POP_APPEND,
    PGC_PSM_POP_CONCAT

    /* New operators...
     * 
     * // capturing strings, u64s, and utf32s 
     *
     * let example_parser = $(identifier) #16(number) `(symbol);
     *
     * // Both of these parsers save head + tail
     * // the first one concats its result into the saved list.
     * // the second one appends the result as a full list.
     *
     * let another_example = "details:" >(shallow_term) ',' <(deep_expr);
     *
     * // the last new parser saves all for full rollback on failure, typically
     * // used within branches of a sum/choice combinator.
     *
     * let last_example = ^(this_complex_expr) | ^(that_complex_expr); 
     *
     * let this_example = $iden :MY_TYPE
     */
};

/*
 * 
 *
 *
 */

#endif
