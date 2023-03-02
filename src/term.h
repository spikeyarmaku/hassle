#ifndef _TERM_H_
#define _TERM_H_

#include "expr.h"
#include "rational.h"

struct Env; // Forward declaration

enum TermType {ValTerm, ExprTerm, AbsTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string;
        struct Rational rational;
    };
};

struct Term {
    enum TermType type;
    union {
        struct Value value;
        struct Expr expr;
        struct Term (*apply)(struct Env*, struct Expr);
    };
};

struct Term make_number (struct Rational);
struct Term make_string (char*);
struct Term make_expr   (struct Expr);

#endif