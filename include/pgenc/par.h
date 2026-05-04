#ifndef PGENC_PAR_H
#define PGENC_PAR_H

#include "pgenc/buf.h"
#include "pgenc/stk.h"

/** Parser Combinator */
struct pgc_par;

/** Custom Parser Hook */
typedef int (*pgc_par_hook_t)(
    struct pgc_buf *buffer,
    void *state);

/** Custom Parser Call */
typedef int (*pgc_par_call_t)(
    const struct pgc_par *callee,
    struct pgc_buf *buffer,
    struct pgc_stk *stack,
    void *state);

/** Parser Type Discriminator */
enum pgc_par_tag {
    PGC_PAR_CMP,                            /** Comparison Parser */
    PGC_PAR_UTF8,                           /** UTF8 Parser */
    PGC_PAR_BYTE,                           /** Byte Parser */
    PGC_PAR_SET,                            /** Charset Parser */
    PGC_PAR_AND,                            /** Product Parser */
    PGC_PAR_OR,                             /** Choice Parser */
    PGC_PAR_REP,                            /** Repetition Parser */
    PGC_PAR_HOOK,                           /** Hook Parser */
    PGC_PAR_CALL                            /** Custom Call */
};

struct pgc_par {
    enum pgc_par_tag tag;
    union {
        struct {
            struct pgc_par *arg1;
            struct pgc_par *arg2;
        } pair;
        struct {
            struct pgc_par *sub;
            uint32_t min;
            uint32_t max;
        } trip;
        struct {
            const char *value;
            size_t length;
        } str;
        struct {
            pgc_par_call_t caller; 
            struct pgc_par *callee;
        } call;
        struct {
            struct pgc_utf8_range *ranges;
            size_t nranges;
        } utf8;
        pgc_par_hook_t hook;
        struct pgc_cset *set;
        int byte;
    } u;
};

#define PGC_PAR_CMP(STR, LEN) { \
    .tag = PGC_PAR_CMP, \
    .u.str.val = STR, \
    .u.str.len = LEN }

#define PGC_PAR_BYTE(OCTET) { \
    .tag = PGC_PAR_BYTE, \
    .u.byte = OCTET }

#define PGC_PAR_UTF8(MIN, MAX) { \
    .tag = PGC_PAR_UTF8, \
    .u.trip.min = MIN, \
    .u.trip.max = MAX }

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
    .u.trip.sub = SUB, \
    .u.trip.min = MIN, \
    .u.trip.max = MAX }

#define PGC_PAR_HOOK(CALLBACK) { \
    .tag = PGC_PAR_HOOK, \
    .u.hook = CALLBACK }

#define PGC_PAR_CALL(FUN, VAR) { \
    .tag = PGC_PAR_CALL, \
    .u.call.fun = FUN, \
    .u.call.var = VAR }

struct pgc_par_frame {
    size_t step;
    uint64_t offset;
};

/** Push parser to stack.  Returns: PGC_OK | PGC_SOFLO | PGC_UNRCH. */
static int pgc_par_push(const struct pgc_par *par, struct pgc_stk *stk) 
{
    struct pgc_par_frame *frame = NULL;
    const struct pgc_par **ptr = NULL;
    switch (par->tag) {
        case PGC_PAR_BYTE: 
        case PGC_PAR_UTF8:
        case PGC_PAR_SET:
        case PGC_PAR_CMP: 
        case PGC_PAR_HOOK: 
        case PGC_PAR_CALL:
            if (!(ptr = pgc_stk_push(stk, sizeof(*ptr))))
                return PGC_SOFLO;
            *ptr = par;
            return PGC_OK;
        case PGC_PAR_AND: 
        case PGC_PAR_OR: 
        case PGC_PAR_REP: 
            if (!(frame = pgc_stk_push(stk, sizeof(*frame))))
                return PGC_SOFLO;
            *frame = (struct pgc_par_frame){ 0, 0 };
            if (!(ptr = pgc_stk_push(stk, sizeof(*ptr)))) {
                (void)pgc_stk_pop(stk, sizeof(*frame));
                return PGC_SOFLO;
            }
            *ptr = par;
            return PGC_OK;
        default: 
            pgc_panic("unreachable state"); 
            return PGC_UNRCH;
    }
}

static inline void pgc_par_pop_ptr(struct pgc_stk *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(void*)));
}

static inline void pgc_par_pop_frame(struct pgc_stk *stk)
{
    pgc_always(pgc_stk_pop(stk, sizeof(void*)));
    pgc_always(pgc_stk_pop(stk, sizeof(struct pgc_par_frame)));
}

static inline int pgc_par_run_byte(
    const struct pgc_par *par, 
    struct pgc_buf *buf)
{
    return pgc_buf_match_char(buf, par->u.byte); 
}

static inline int pgc_par_run_set(
    const struct pgc_par *par,
    struct pgc_buf *buf)
{
    return pgc_buf_match_set(buf, par->u.set);
}

static inline int pgc_par_run_utf8(
    const struct pgc_par *par,
    struct pgc_buf *buf)
{
    return pgc_buf_match_utf8(buf, par->u.utf8.ranges, par->u.utf8.nranges);
}

static inline int pgc_par_run_cmp(
    const struct pgc_par *par,
    struct pgc_buf *buf)
{
    return pgc_buf_match_str(buf, par->u.str.value, par->u.str.length);
}

static inline int pgc_par_run_hook(
    const struct pgc_par *par,
    struct pgc_buf *buf,
    void *state)
{
    return par->u.hook(buf, state);
}

static inline int pgc_par_run_call(
    const struct pgc_par *par,
    struct pgc_buf *buf,
    struct pgc_stk *stk,
    void *state)
{
    return par->u.call.caller(par->u.call.callee, buf, stk, state);
}

static int pgc_par_run_and(
    const struct pgc_par *par,
    struct pgc_stk *stk,
    const int status)
{
    struct pgc_par_frame *frame = pgc_stk_peek(stk, sizeof(void*));
    assert(frame != NULL); 
    switch (frame->step) {
        case 0:
            if (pgc_par_push(par->u.pair.arg1, stk) != PGC_OK) {
                (void)pgc_par_pop_frame(stk);
                return PGC_SOFLO;
            }
            frame->step = 1;
            return PGC_OK;
        case 1:
            (void)pgc_par_pop_frame(stk);
            if (status != PGC_OK) return status;
            pgc_always(pgc_par_push(par->u.pair.arg2, stk));
            return PGC_OK;
        default:
            pgc_panic("unreachable state"); 
            return PGC_UNRCH; 
    }
}

static int pgc_par_run_or(
    const struct pgc_par *par,
    struct pgc_buf *buf,
    struct pgc_stk *stk,
    const int status)
{
    struct pgc_par_frame *frame = pgc_stk_peek(stk, sizeof(void*));
    assert(frame != NULL);
    switch (frame->step) {
        case 0:
            if (pgc_par_push(par->u.pair.arg1, stk) != PGC_OK) {
                (void)pgc_par_pop_frame(stk);
                return PGC_SOFLO;
            }
            frame->offset = pgc_buf_tell(buf);
            frame->step = 1;
            return PGC_OK;
        case 1:
            (void)pgc_par_pop_frame(stk);
            switch (status) {
                case PGC_OK: 
                    return PGC_OK;
                case PGC_NOMAT:
                case PGC_BUFLO: 
                    pgc_try(pgc_buf_seek(buf, frame->offset));
                    pgc_always(pgc_par_push(par->u.pair.arg2, stk));
                    return PGC_OK;
                default:
                    return status;
            }
        default:
            pgc_panic("unreachable state");
            return PGC_UNRCH;
    }
}

static int pgc_par_run_rep(
    const struct pgc_par *par,
    struct pgc_buf *buf,
    struct pgc_stk *stk,
    const int status) 
{
    struct pgc_par_frame *frame = pgc_stk_peek(stk, sizeof(void*));
    assert(frame);
    const uint32_t max = par->u.trip.max;
    const uint32_t min = par->u.trip.min;
    assert(min <= max);
    if (frame->step == 0) goto NEXT;
    switch (status) {
        case PGC_NOMAT:
        case PGC_BUFLO:
            (void)pgc_par_pop_frame(stk);
            if (min < frame->step) {
                pgc_try(pgc_buf_seek(buf, frame->offset));
                return PGC_OK;
            } else return status;
        case PGC_OK:
            if (frame->step < max) goto NEXT;
        default:
            (void)pgc_par_pop_frame(stk);
            return status;
    }
    NEXT:
    if (pgc_par_push(par->u.trip.sub, stk)) {
        (void)pgc_par_pop_frame(stk);
        return PGC_SOFLO;
    }
    frame->offset = pgc_buf_tell(buf);
    frame->step += 1;
    return PGC_OK;
}

/**
 * Run a parser by taking a buffer, a stack, and a state.  Returns a negative 
 * error code on failure, otherwise PGC_ERR_OK.
 */
static inline int pgc_par_run(
    const struct pgc_par *par, 
    struct pgc_buf *buf,
    struct pgc_stk *stk,
    void *st)
{
    int status;
    const void* base = pgc_stk_peek(stk, 0);
    pgc_try(pgc_par_push(par, stk));
}

#endif
