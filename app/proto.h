#ifndef PGENC_PROTO_H
#define PGENC_PROTO_H

#include "pgenc/ast.h"

/** Generate a syntax prototype for parsing PGENC grammars. */

struct pgc_ast *pgc_lang_proto(void);

#endif
