#include "term.h"

BOOL term_is_equal(struct Term t1, struct Term t2) {
    if (t1.type == t2.type) {
        switch (t1.type) {
            case AbsTerm: {
                return t1.abs.apply == t2.abs.apply &&
                    t1.abs.closure == t2.abs.closure;
            }
            case ValTerm: {
                if (t1.value.type == t2.value.type) {
                    if (t1.value.type == RationalVal) {
                        return rational_is_equal(t1.value.rational,
                            t2.value.rational);
                    } else {
                        return strcmp(t1.value.string, t2.value.string) == 0;
                    }
                } else {
                    return FALSE;
                }
            }
            case ExprTerm: {
                return is_equal_expr(t1.expr, t2.expr);
            }
            default: {
                return FALSE;
            }
        }
    } else {
        return FALSE;
    }
}

struct Term term_make_number(Rational_t r) {
    struct Value v;
    v.type = RationalVal;
    v.rational = r;
    struct Term t;
    t.type = ValTerm;
    t.value = v;
    return t;
}

struct Term term_make_string(char* str) {
    struct Value v;
    v.type = StringVal;
    v.string = str; // Don't copy strings if it isn't necessary
    struct Term t;
    t.type = ValTerm;
    t.value = v;
    return t;
}

struct Term term_make_expr(Expr_t expr) {
    struct Term t;
    t.type = ExprTerm;
    t.expr = expr;
    return t;
}

void term_free(struct Term t) {
    switch (t.type) {
        case AbsTerm : {
            free_mem("free_term/closure", t.abs.closure);
            break;
        }
        case ValTerm: {
            if (t.value.type == RationalVal) {
                debug(0, "free_term: freeing up rational\n");
                rational_free(t.value.rational);
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

char* term_to_string(struct Term t) {
    switch (t.type) {
        case AbsTerm: {
            char* str = "<Function>";
            char* result = (char*)allocate_mem("term_print/function", NULL,
                sizeof(char) * (strlen(str) + 1));
            strcpy(result, str);
            return result;
        }
        case ValTerm: {
            switch (t.value.type) {
                case RationalVal: {
                    return rational_to_string(t.value.rational);
                }
                case StringVal: {
                    char* result = (char*)allocate_mem("term_print/string",
                        NULL, sizeof(char) * (strlen(t.value.string) + 1));
                    strcpy(result, t.value.string);
                    return result;
                }
            }
            break;
        }
        case ExprTerm: {
            return expr_to_string(t.expr);
        }
    }
}
