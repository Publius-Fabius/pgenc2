#ifndef PGENC_GEN_H
#define PGENC_GEN_H

#include "pgenc/ast.h"
#include <stdio.h>

/** Generate PGENC parsers in standard C. */
int pgc_lang_gen(
    FILE *out,
    struct pgc_ast_lst *list,
    const char *prefix);

#endif
