#ifndef PGENC_AST_H
#define PGENC_AST_H

#include <stdint.h>

/** AST Tag */ 
enum pgc_ast_tag {                              
    PGC_AST_U64,                                /** 64bit Unsigned Int */
    PGC_AST_UTF,                                /** 32bit UTF Value */
    PGC_AST_STR,                                /** String Node */
    PGC_AST_LST                                 /** List Node */
};

struct pgc_ast_lst;

/** Syntax Tree */ 
typedef struct pgc_ast {                                
    int32_t atag;                               /** AST tag */
    int32_t utag;                               /** User tag */
    union {
        uint64_t u64;                           /** 64bit Value */
        char *str;                              /** String Value */
        struct pgc_ast_lst *lst;                /** List Value */
    } u;
} pgc_ast_t;

/** AST Linked List Node */
typedef struct pgc_ast_lst {                                
    pgc_ast_t val;                              /** Value */
    struct pgc_ast_lst *nxt;                    /** Next node */
} pgc_ast_lst_t;

static inline void pgc_ast_init_u64(
    pgc_ast_t *a, 
    const int32_t utag, 
    const uint64_t v)
{
    a->atag = PGC_AST_U64;
    a->utag = utag;
    a->u.u64 = v;
}

static inline void pgc_ast_init_str(
    pgc_ast_t *a, 
    const int32_t utag, 
    char* v)
{
    a->atag = PGC_AST_STR;
    a->utag = utag;
    a->u.str = v;
} 

static inline void pgc_ast_init_lst(
    pgc_ast_t *a, 
    const int32_t utag, 
    pgc_ast_lst_t *lst)
{
    a->atag = PGC_AST_LST;
    a->utag = utag;
    a->u.lst = lst;
}

#endif
