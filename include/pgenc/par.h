#ifndef PGENC_PAR_H
#define PGENC_PAR_H

#include "pgenc/buf.h"
#include "pgenc/psm.h"
#include "pgenc/stk.h"
#include "pgenc/codec.h"

/** Parser Combinator Type */
enum pgc_par_tag {

    /* Pure Combinators */

    PGC_PAR_BYTE,                       /** Byte Equality Parser */
    PGC_PAR_SET,                        /** U8set Membership Parser */
    PGC_PAR_CMP,                        /** Raw Comparison (memcmp) Parser */
    PGC_PAR_UTF8,                       /** UTF8 "In Range" Parser */
    PGC_PAR_AND,                        /** Product Parser */
    PGC_PAR_OR,                         /** Choice Parser */
    PGC_PAR_REP,                        /** Repetition Parser */

    /* Stateful Combinators */
    
    PGC_PAR_STR,                        /** Copy & Attach NULL Delimited Str */
    PGC_PAR_NUM,                        /** Decode & Attach Number */
    PGC_PAR_NEST,                       /** Push List & Nest Result */
    PGC_PAR_UTAG                        /** Set PSM->utag */
};

typedef struct pgc_par_decrec {
    size_t base;
    const pgc_decoder_t *dec;
} pgc_par_decrec_t;

typedef struct pgc_par {
    int tag;
    union {
        struct {
            struct pgc_par *arg1;
            struct pgc_par *arg2;
        } pair;
        struct {
            struct pgc_par *sub;
            uint32_t min;
            uint32_t max;
        } rep;
        struct {
            const char *val;
            size_t len;
        } str;
        struct {
            struct pgc_utf8_range *ranges;
            size_t num_ranges;
        } utf8;
        struct {
            struct pgc_par *digs;
            struct pgc_par_decrec *rec; 
        } num;
        struct pgc_par *par;
        struct pgc_cset *set;
        int utag;
        int byte;
    } u;
} pgc_par_t;

#define PGC_PAR_CMP(STR, LEN) { \
    .tag = PGC_PAR_CMP, \
    .u.str.val = STR, \
    .u.str.len = LEN }

#define PGC_PAR_BYTE(OCTET) { \
    .tag = PGC_PAR_BYTE, \
    .u.byte = OCTET }

#define PGC_PAR_UTF8(RANGES, NRANGES) { \
    .tag = PGC_PAR_UTF8, \
    .u.utf8.ranges = RANGES, \
    .u.utf8.num_ranges = NRANGES, } \

#define PGC_PAR_SET(SET) { \
    .tag = PGC_PAR_SET, \
    .u.set = SET }

#define PGC_PAR_AND(ARG1, ARG2) { \
    .tag = PGC_PAR_AND, \
    .u.pair.arg1 = ARG1, \
    .u.pair.arg2 = ARG2 }

#define PGC_PAR_OR(ARG1, ARG2) { \
    .tag = PGC_PAR_OR, \
    .u.pair.arg1 = ARG1, \
    .u.pair.arg2 = ARG2 }

#define PGC_PAR_REP(SUB, MIN, MAX) { \
    .tag = PGC_PAR_REP, \
    .u.rep.sub = SUB, \
    .u.rep.min = MIN, \
    .u.rep.max = MAX }

#define PGC_PAR_STR(PAR) { \
    .tag = PGC_PAR_STR, \
    .u.par = PAR }

#define PGC_PAR_NUM(PAR, REC) { \
    .tag = PGC_PAR_NUM, \
    .u.num.digs = PAR, \
    .u.num.rec = REC }

#define PGC_PAR_NEST(PAR) { \
    .tag = PGC_PAR_NEST, \
    .u.par = PAR }

#define PGC_PAR_UTAG(UTAG) { \
    .tag = PGC_PAR_UTAG, \
    .u.utag = UTAG }

/** And Parser Frame */
typedef struct pgc_par_sframe {
    size_t step;
} pgc_par_sframe_t;

/** Parser Stack Frame */
typedef struct pgc_par_oframe {
    size_t step;
    uint64_t offset;
} pgc_par_oframe_t;

/** Parser List Frame */
typedef struct pgc_par_lframe {
    size_t step;
    pgc_ast_lst_t *first;
    pgc_ast_lst_t *last;
} pgc_par_lframe_t;

static int pgc_par_push_ptr(const pgc_par_t *par, pgc_stk_t *stk) 
{
    const pgc_par_t **ptr = pgc_stk_push(stk, sizeof(*ptr));
    if (ptr == NULL) return PGC_SOFLO;
    *ptr = par;
    return PGC_OK;
}

static void pgc_par_pop_ptr(pgc_stk_t *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(const pgc_par_t*)));
}

static int pgc_par_push_sframe(const pgc_par_t *par, pgc_stk_t *stk)
{
    pgc_par_sframe_t *sframe = pgc_stk_push(stk, sizeof(*sframe));
    if (sframe == NULL) return PGC_SOFLO;
    (void)memset(sframe, 0, sizeof(*sframe));
    if (pgc_par_push_ptr(par, stk) == PGC_SOFLO) {
        pgc_always(pgc_stk_pop(stk, sizeof(*sframe)));
        return PGC_SOFLO;
    }
    return PGC_OK; 
}

static void pgc_par_pop_sframe(pgc_stk_t *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(const pgc_par_t*)));
    pgc_always(pgc_stk_pop(stk, sizeof(pgc_par_sframe_t)));
}

static int pgc_par_push_oframe(const pgc_par_t *par, pgc_stk_t *stk)
{
    pgc_par_oframe_t *oframe = pgc_stk_push(stk, sizeof(*oframe));
    if (oframe == NULL) return PGC_SOFLO;
    (void)memset(oframe, 0, sizeof(*oframe));
    if (pgc_par_push_ptr(par, stk) == PGC_SOFLO) {
        pgc_always(pgc_stk_pop(stk, sizeof(*oframe)));
        return PGC_SOFLO;
    }
    return PGC_OK; 
}

static void pgc_par_pop_oframe(pgc_stk_t *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(const pgc_par_t*)));
    pgc_always(pgc_stk_pop(stk, sizeof(pgc_par_oframe_t)));
}

static int pgc_par_push_lframe(const pgc_par_t *par, pgc_stk_t *stk)
{
    pgc_par_lframe_t *lframe = pgc_stk_push(stk, sizeof(*lframe));
    if (lframe == NULL) return PGC_SOFLO;
    (void)memset(lframe, 0, sizeof(*lframe));
    if (pgc_par_push_ptr(par, stk) == PGC_SOFLO) {
        pgc_always(pgc_stk_pop(stk, sizeof(*lframe)));
        return PGC_SOFLO;
    }
    return PGC_OK; 
}

static void pgc_par_pop_lframe(pgc_stk_t *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(const pgc_par_t*)));
    pgc_always(pgc_stk_pop(stk, sizeof(pgc_par_lframe_t)));
}

/** Push parser to stack.  Returns: PGC_OK | PGC_SOFLO | PGC_UNRCH. */
static int pgc_par_push(const pgc_par_t *par, pgc_stk_t *stk) 
{
    switch (par->tag) {
        case PGC_PAR_BYTE: 
        case PGC_PAR_UTF8:
        case PGC_PAR_SET:
        case PGC_PAR_CMP: 
        case PGC_PAR_UTAG:
            return pgc_par_push_ptr(par, stk);
        case PGC_PAR_AND: 
            return pgc_par_push_sframe(par, stk); 
        case PGC_PAR_OR: 
        case PGC_PAR_REP:
        case PGC_PAR_NUM:
        case PGC_PAR_STR:
            return pgc_par_push_oframe(par, stk);
        case PGC_PAR_NEST:
            return pgc_par_push_lframe(par, stk);
        default: 
            pgc_panic("unreachable state"); 
            return PGC_UNRCH;
    }
}

static int pgc_par_run_byte(
    const pgc_par_t *par, 
    pgc_buf_t *buf,
    pgc_stk_t *stk)
{
    (void)pgc_par_pop_ptr(stk);
    return pgc_buf_match_char(buf, par->u.byte);
}

static int pgc_par_run_set(
    const pgc_par_t *par, 
    pgc_buf_t *buf,
    pgc_stk_t *stk)
{
    (void)pgc_par_pop_ptr(stk);
    return pgc_buf_match_set(buf, par->u.set);
}

static int pgc_par_run_utf8(
    const pgc_par_t *par, 
    pgc_buf_t *buf,
    pgc_stk_t *stk)
{
    (void)pgc_par_pop_ptr(stk);
    return pgc_buf_match_utf8(
        buf, 
        par->u.utf8.ranges, 
        par->u.utf8.num_ranges);
}

static int pgc_par_run_cmp(
    const pgc_par_t *par, 
    pgc_buf_t *buf,
    pgc_stk_t *stk)
{
    (void)pgc_par_pop_ptr(stk);
    return pgc_buf_match_str(buf, par->u.str.val, par->u.str.len);
}

static int pgc_par_run_utag(
    const pgc_par_t *par,
    pgc_stk_t *stk,
    pgc_psm_t *psm)
{
    (void)pgc_par_pop_ptr(stk);
    psm->utag = par->u.utag;
    return PGC_OK;
}

static int pgc_par_run_and(
    const pgc_par_t *par,
    pgc_stk_t *stk,
    int status)
{
    pgc_par_sframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame != NULL); 
    switch (frame->step) {
        case 0:
            if ((status = pgc_par_push(par->u.pair.arg1, stk)) != PGC_OK) 
                goto CLEANUP;            
            frame->step = 1;
            return PGC_OK;
        case 1:
            if (status != PGC_OK) goto CLEANUP;
            (void)pgc_par_pop_sframe(stk);
            return pgc_par_push(par->u.pair.arg2, stk);
        default:
            pgc_panic("unreachable state"); 
            return PGC_UNRCH; 
    }
CLEANUP:
    (void)pgc_par_pop_sframe(stk);
    return status;
} 

static int pgc_par_run_or(
    const pgc_par_t *par,
    pgc_buf_t *buf,
    pgc_stk_t *stk,
    int status)
{
    pgc_par_oframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame != NULL);
    switch (frame->step) {
        case 0:
            if ((status = pgc_par_push(par->u.pair.arg1, stk)) != PGC_OK)
                goto CLEANUP;
            frame->offset = pgc_buf_tell(buf);
            frame->step = 1;
            return PGC_OK;
        case 1:
            switch (status) {
                case PGC_IUTF8:
                case PGC_NOMAT:
                case PGC_BUFLO:
                    if ((status = pgc_buf_seek(buf, frame->offset)) != PGC_OK)
                        goto CLEANUP;
                    (void)pgc_par_pop_oframe(stk);
                    return pgc_par_push(par->u.pair.arg2, stk);
                default:
                    goto CLEANUP;
            }
        default:
            pgc_panic("unreachable state");
            return PGC_UNRCH;
    }
CLEANUP:
    (void)pgc_par_pop_oframe(stk);
    return status;
}

static int pgc_par_run_rep(
    const pgc_par_t *par,
    pgc_buf_t *buf,
    pgc_stk_t *stk,
    int status) 
{
    pgc_par_oframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame && par->u.rep.min <= par->u.rep.max);
    if (frame->step > 0) switch (status) {
        case PGC_IUTF8:
        case PGC_NOMAT:
        case PGC_BUFLO:
            if (par->u.rep.min < frame->step)
                status = pgc_buf_seek(buf, frame->offset);
            goto CLEANUP;
        case PGC_OK:
            if (frame->step < par->u.rep.max) break;
            goto CLEANUP;
        default:
            goto CLEANUP;
    }
    if ((status = pgc_par_push(par->u.rep.sub, stk)) != PGC_OK)
        goto CLEANUP;
    frame->offset = pgc_buf_tell(buf);
    frame->step += 1;
    return PGC_OK;
CLEANUP:
    (void)pgc_par_pop_oframe(stk);
    return status;
} 

static int pgc_par_run_str(
    const pgc_par_t *par,
    pgc_buf_t *buf,
    pgc_stk_t *stk,
    pgc_psm_t *psm,
    int status)
{
    pgc_par_oframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame != NULL); 
    switch (frame->step) {
        case 0:
            if ((status = pgc_par_push(par->u.par, stk)) != PGC_OK) 
                goto CLEANUP;
            frame->step = 1;
            frame->offset = pgc_buf_tell(buf);
            return PGC_OK;
        case 1:
            if (status != PGC_OK) goto CLEANUP;
            break;
        default:
            pgc_panic("unreachable state"); 
            return PGC_UNRCH; 
    }
    const uint64_t end = pgc_buf_tell(buf);
    assert(frame->offset <= end);
    const uint64_t len = end - frame->offset;
    if ((status = pgc_buf_seek(buf, frame->offset)) != PGC_OK)
        goto CLEANUP;
    (void)pgc_par_pop_oframe(stk);
    char *str = pgc_alloc(psm->alloc, len + 1);
    pgc_ast_lst_t *lst = pgc_alloc(psm->alloc, sizeof(pgc_ast_lst_t));
    if (str == NULL || lst == NULL) return PGC_NOMEM;
    str[len] = 0;
    (void)pgc_buf_get(buf, str, len);
    lst->nxt = NULL; 
    (void)pgc_ast_init_str(&lst->val, psm->utag, str);
    (void)pgc_psm_append(psm, lst);
    return PGC_OK;
CLEANUP:
    (void)pgc_par_pop_oframe(stk);
    return status;
}

static int pgc_par_run_num(
    const pgc_par_t *par,
    pgc_buf_t *buf,
    pgc_stk_t *stk,
    pgc_psm_t *psm,
    int status)
{
    pgc_par_oframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame != NULL);
    switch (frame->step) {
        case 0:
            if ((status = pgc_par_push(par->u.par, stk)) != PGC_OK)
                goto CLEANUP;
            frame->step = 1;
            frame->offset = pgc_buf_tell(buf);
            return PGC_OK;
        case 1:
            if (status != PGC_OK) goto CLEANUP;
            break;
        default:
            pgc_panic("unreachable state");
            return PGC_UNRCH;
    }
    pgc_par_decrec_t *rec = par->u.num.rec;
    const uint64_t end = pgc_buf_tell(buf);
    assert(frame->offset <= end);
    const uint64_t len = end - frame->offset;
    if ((status = pgc_buf_seek(buf, frame->offset)) != PGC_OK)
        goto CLEANUP;
    (void)pgc_par_pop_oframe(stk);
    pgc_ast_lst_t *lst = pgc_alloc(psm->alloc, sizeof(pgc_ast_lst_t));
    if (lst == NULL) return PGC_NOMEM;
    void* addr = pgc_buf_claim(buf, len);
    assert(addr != NULL);
    lst->nxt = NULL; 
    (void)pgc_ast_init_u64(&lst->val, psm->utag, 0);
    (void)pgc_decode(addr, len, rec->base, rec->dec, &lst->val.u.u64);
    (void)pgc_buf_consume(buf, len);
    (void)pgc_psm_append(psm, lst);
    return PGC_OK;
CLEANUP:
    (void)pgc_par_pop_oframe(stk);
    return status;
}

static int pgc_par_run_nest(
    const pgc_par_t *par,
    pgc_stk_t *stk,
    pgc_psm_t *psm,
    int status)
{
    pgc_par_lframe_t *frame = pgc_stk_peek(stk, sizeof(const pgc_par_t*));
    assert(frame != NULL);
    switch (frame->step) {
        case 0:
            if ((status = pgc_par_push(par->u.par, stk)) != PGC_OK)
                goto CLEANUP;
            frame->step = 1;
            frame->first = psm->first;
            frame->last = psm->last;
            return PGC_OK;
        case 1:
            if (status != PGC_OK) goto CLEANUP;
            break;
        default:
            pgc_panic("unreachable state");
            return PGC_UNRCH;
    }
    pgc_ast_lst_t *lst = pgc_alloc(psm->alloc, sizeof(*lst));
    if (lst == NULL) {
        status = PGC_NOMEM;
        goto CLEANUP;
    }
    lst->nxt = NULL;
    (void)pgc_ast_init_lst(&lst->val, psm->utag, psm->first);
    psm->first = frame->first;
    psm->last = frame->last;
    (void)pgc_psm_append(psm, lst);
CLEANUP:
    (void)pgc_par_pop_lframe(stk);
    return status;
}

/**
 * Run a parser by taking a buffer, a stack, and a state.  Returns a negative 
 * error code on failure, otherwise PGC_OK.
 */
static int pgc_par_run(
    const pgc_par_t *par, 
    pgc_buf_t *buf,
    pgc_stk_t *stk,
    pgc_psm_t *psm)
{
    int status = -1;
    const pgc_par_t** ptr = NULL;
    const pgc_par_t** base = pgc_stk_top(stk); 
    pgc_try(pgc_par_push(par, stk));
    while ((ptr = pgc_stk_peek(stk, 0)) && ptr < base) {
        switch ((*ptr)->tag) {
            case PGC_PAR_BYTE:
                status = pgc_par_run_byte(*ptr, buf, stk);
                break;
            case PGC_PAR_UTF8:
                status = pgc_par_run_utf8(*ptr, buf, stk);
                break;
            case PGC_PAR_SET:
                status = pgc_par_run_set(*ptr, buf, stk);
                break;
            case PGC_PAR_CMP:
                status = pgc_par_run_cmp(*ptr, buf, stk);
                break;
            case PGC_PAR_AND:
                status = pgc_par_run_and(*ptr, stk, status);
                break;
            case PGC_PAR_OR:
                status = pgc_par_run_or(*ptr, buf, stk, status);
                break;
            case PGC_PAR_REP:
                status = pgc_par_run_rep(*ptr, buf, stk, status);
                break;
            case PGC_PAR_STR:
                status = pgc_par_run_str(*ptr, buf, stk, psm, status);
                break;
            case PGC_PAR_NUM:
                status = pgc_par_run_num(*ptr, buf, stk, psm, status);
                break;
            case PGC_PAR_NEST:
                status = pgc_par_run_nest(*ptr, stk, psm, status);
                break;
            case PGC_PAR_UTAG:
                status = pgc_par_run_utag(*ptr, stk, psm);
                break;
            default:
                pgc_panic("unreachable state");
                return PGC_UNRCH;
        }
    }
    return status;
}

static inline intptr_t pgc_par_runs(
    const pgc_par_t *par,
    const char *str,
    pgc_stk_t *stk,
    pgc_psm_t *psm)
{
    const size_t len = strlen(str);
    pgc_buf_t buf;
    (void)pgc_buf_init(&buf, (void*)str, len);
    (void)pgc_buf_advance(&buf, len);
    const int res = pgc_par_run(par, &buf, stk, psm);
    return res == PGC_OK ? (intptr_t)pgc_buf_tell(&buf) : res;
}

#endif
