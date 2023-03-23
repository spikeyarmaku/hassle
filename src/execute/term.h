#ifndef _TERM_H_
#define _TERM_H_

#include "parse\expr.h"
#include "rational\rational.h"

#include "memory.h"

struct EnvFrame; // Forward declaration

enum TermType {ValTerm, ExprTerm, AbsTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string;
        Rational_t rational;
    };
};

typedef ErrorCode_t Apply_t(struct EnvFrame*, Expr_t, void*, struct Term*);
struct Abstraction {
    Apply_t* apply;
    void* closure;
};

struct Term {
    enum TermType type;
    union {
        struct Value value;
        Expr_t expr;
        struct Abstraction abs;
    };
};

BOOL        term_is_equal       (struct Term, struct Term);

struct Term term_make_number    (Rational_t);
struct Term term_make_string    (char*);
struct Term term_make_expr      (Expr_t);

void        term_free           (struct Term);

char*       term_to_string      (struct Term);

#endif