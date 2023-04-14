// Values belong to `Closure`s, `Closure`s belong to `Term`s, and `Term`s belong
// to either a frame or to the result

#ifndef _TERM_H_
#define _TERM_H_

#include "parse\expr.h"
#include "rational\rational.h"

#include "global.h"
#include "memory.h"

struct EnvFrame; // Forward declaration

typedef struct Term* Term_t;

enum TermType {ValTerm, ExprTerm, AbsTerm}; // , ErrTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string;
        Rational_t rational;
    };
};

typedef void ClosureFree_t(void*);
struct Closure {
    size_t size;
    void* data;
    ClosureFree_t *closure_free;
};

typedef Term_t Apply_t(struct EnvFrame*, Expr_t, struct Closure);
struct Abstraction {
    Apply_t* apply;
    struct Closure closure;
};

BOOL                term_is_equal       (Term_t, Term_t);

Term_t              term_make_number    (Rational_t);
Term_t              term_make_string    (char*);
Term_t              term_make_expr      (Expr_t);
Term_t              term_make_abs       (Apply_t, void*, size_t, ClosureFree_t);

enum TermType       term_get_type       (Term_t);
struct Value        term_get_value      (Term_t);
Expr_t              term_get_expr       (Term_t);
struct Abstraction  term_get_abs        (Term_t);

void                term_free           (Term_t*);
Term_t              term_copy           (Term_t);

char*               term_to_string      (Term_t);
void                term_print          (Term_t);

// struct Closure  closure_copy    (struct Closure);

#endif