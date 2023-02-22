#ifndef _TERM_H_
#define _TERM_H_

#include "env.h"
#include "expr.h"

enum TermType {ValTerm, ExprTerm, AbsTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string; // must be null-terminated!
        struct Rational* rational;
    };
};

struct Term {
    enum TermType type;
    union {
        struct Term* (*apply)(struct Env*, struct Expr*);
        struct Expr* expr;
    };
};

#endif