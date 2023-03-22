#ifndef _EXPR_H_
#define _EXPR_H_

#include <string.h>

#include "config.h"
#include "global.h"

#include "memory.h"

/*
Expr_t usage:

Construction:
  To construct an Expr_t, an ExprBuilder_t must be used. One can be created by
  calling `make_expr_builder`, optionally giving it a dict (this allows a dict
  to be reused between multiple parsers). If no dict is given, an empty one will
  be created. Then, tokens can be added with `append_token`, and after the last
  one is added, `finalize_builder` must be called, so that the inner dict and
  expr won't take up more space than necessary.

Destruction:
  ExprBuilder_t does not need to be destructed after use, but the expr and dict
  components need to be freed up if they are not in use anymore. Destroying an
  expr is done via `free_expr`.
*/

enum ExprType {ExprAtom, ExprList};

// https://benpaulhanna.com/writing-a-game-in-c-parsing-s-expressions.html
struct Expr {
  struct Expr* _next;
//   struct Expr* _prev;
  enum ExprType _type;
  union {
    struct Expr* _list;
    char* _symbol;
  };
};
typedef struct Expr* Expr_t;

Expr_t  _expr_make_empty        (ErrorCode_t*);
void    _expr_set_as_atom       (Expr_t, char*);
void    _expr_set_as_list       (Expr_t);

void    expr_add_to_list        (ErrorCode_t*, Expr_t list, Expr_t child);
Expr_t  expr_make_atom          (ErrorCode_t*, char*);
Expr_t  expr_make_empty_list    (ErrorCode_t*);

Expr_t  expr_get_next           (Expr_t);
// Expr_t  expr_get_prev           (Expr_t);
BOOL    expr_is_list            (Expr_t);
Expr_t  expr_get_list           (Expr_t);
char*   expr_get_symbol         (Expr_t);

BOOL    expr_is_equal           (Expr_t, Expr_t);

char*   expr_to_string          (Expr_t);

void    expr_free               (Expr_t*);

#endif