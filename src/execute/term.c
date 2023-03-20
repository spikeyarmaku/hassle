#include "term.h"

struct Term make_number(Rational r) {
    struct Value v;
    v.type = RationalVal;
    v.rational = r;
    struct Term t;
    t.type = ValTerm;
    t.value = v;
    return t;
}

struct Term make_string(char* str) {
    struct Value v;
    v.type = StringVal;
    v.string = str; // Don't copy strings if it isn't necessary
    struct Term t;
    t.type = ValTerm;
    t.value = v;
    return t;
}

struct Term make_expr(Expr expr) {
    struct Term t;
    t.type = ExprTerm;
    t.expr = expr;
    return t;
}