#ifndef _EXPR_H_
#define _EXPR_H_

#include <string.h>
#include <stdio.h>

#include "config.h"
#include "global.h"

#include "memory.h"

/*
Expr_t usage:

Construction:
  Use `expr_make_atom` or `expr_make_empty_list`. Adding children to a list is
  done with `expr_add_to_list`.

Destruction:
  Destroying an expr is done with `expr_free`.
*/

enum ExprType {ExprAtom, ExprList};

typedef struct Expr* Expr_t;

void        expr_add_to_list        (Expr_t list, Expr_t child);
Expr_t      expr_make_atom          (char*);
Expr_t      expr_make_empty_list    ();

Expr_t      expr_get_child          (Expr_t, size_t);
size_t      expr_get_child_count    (Expr_t);
BOOL        expr_is_list            (Expr_t);
BOOL        expr_is_empty_list      (Expr_t);
Expr_t*     expr_get_list           (Expr_t);
char*       expr_get_symbol         (Expr_t);

BOOL        expr_is_equal           (Expr_t, Expr_t);
size_t      expr_match_size         (Expr_t, Expr_t);

char*       expr_to_string          (Expr_t);
void        expr_print              (Expr_t);

void        expr_free               (Expr_t*);
Expr_t      expr_copy               (Expr_t);

#endif