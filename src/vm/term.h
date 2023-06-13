#ifndef _TERM_H_
#define _TERM_H_

#include <stdlib.h>

#include "parse\expr.h"
#include "primval.h"

enum TermType   {PrimvalTerm, AbsTerm, SyntaxTerm, LazyAppTerm, StrictAppTerm,
                OpTerm, WorldTerm};
enum PrimOp     {Lambda, Eval, Add, Sub, Mul, Div, Eq};

typedef struct Term Term_t;

Term_t*         term_from_expr          (Expr_t*);
Term_t*         term_make_primval       (PrimVal_t*);
Term_t*         term_make_abs           (char*, Term_t*);
Term_t*         term_make_syntax        (char*, Term_t*);
Term_t*         term_make_lazy_app      (Term_t*, Term_t*);
Term_t*         term_make_strict_app    (Term_t*, Term_t*);
enum TermType   term_get_type           (Term_t*);
PrimVal_t*      term_get_primval        (Term_t*);
char*           term_get_abs_var        (Term_t*);
Term_t*         term_get_abs_body       (Term_t*);
Term_t*         term_get_app_term1      (Term_t*);
Term_t*         term_get_app_term2      (Term_t*);
void            term_free               (Term_t*);
void            term_print              (Term_t*);

#endif