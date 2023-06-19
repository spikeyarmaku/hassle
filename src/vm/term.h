#ifndef _TERM_H_
#define _TERM_H_

#include <stdlib.h>

#include "parse\expr.h"
#include "primval.h"
#include "primop.h"

enum TermType   {PrimvalTerm, AbsTerm, LazyAppTerm, StrictAppTerm, OpTerm,
                WorldTerm};

typedef struct Term Term_t;

Term_t*         term_from_expr          (Expr_t*);
Term_t*         term_make_primval       (PrimVal_t*);
Term_t*         term_make_abs           (char*, Term_t*);
// Term_t*         term_make_syntax        (char*, Term_t*);
Term_t*         term_make_lazy_app      (Term_t*, Term_t*);
Term_t*         term_make_strict_app    (Term_t*, Term_t*);
Term_t*         term_make_op            (enum PrimOp);
enum TermType   term_get_type           (Term_t*);
PrimVal_t*      term_get_primval        (Term_t*);
char*           term_get_abs_var        (Term_t*);
Term_t*         term_get_abs_body       (Term_t*);
Term_t*         term_get_app_term1      (Term_t*);
Term_t*         term_get_app_term2      (Term_t*);
enum PrimOp     term_get_op             (Term_t*);
Term_t*         term_copy               (Term_t*);
void            term_free               (Term_t*);
void            term_free_toplevel      (Term_t*);
void            term_print              (Term_t*);

// Helper functions to make operator `eq` and primitive form `SyntaxTerm` work
Term_t*         term_make_cons          ();
Term_t*         term_make_nil           ();
Term_t*         term_make_head          ();
Term_t*         term_make_tail          ();
Term_t*         term_make_true          ();
Term_t*         term_make_false         ();
Term_t*         term_encode_as_list     (Term_t*);
// Term_t*         term_decode_from_list   (Term_t*);

#endif