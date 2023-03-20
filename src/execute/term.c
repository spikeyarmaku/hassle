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

void free_term(struct Term t) {
    switch (t.type) {
        case AbsTerm : {
            free_mem("free_term/closure", t.abs.closure);
            break;
        }
        case ValTerm: {
            if (t.value.type == RationalVal) {
                free_rational(t.value.rational);
            } else {
                free_mem("free_term/string", t.value.string);
            }
            break;
        }
        case ExprTerm: {
            break;
        }
    }
}
