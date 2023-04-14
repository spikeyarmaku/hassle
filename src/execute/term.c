#include "term.h"

struct Term {
    enum TermType type;
    union {
        struct Value        value;
        Expr_t              expr;
        struct Abstraction  abs;
        // char*               err;
    };
};

BOOL term_is_equal(Term_t t1, Term_t t2) {
    if (t1 == NULL && t2 == NULL) return TRUE;
    if (t1 == NULL || t2 == NULL) return FALSE;

    if (t1->type == t2->type) {
        switch (t1->type) {
            case AbsTerm: {
                return t1->abs.apply == t2->abs.apply &&
                    t1->abs.closure.size == t2->abs.closure.size &&
                    t1->abs.closure.data == t2->abs.closure.data;
            }
            case ValTerm: {
                if (t1->value.type == t2->value.type) {
                    if (t1->value.type == RationalVal) {
                        return rational_is_equal(t1->value.rational,
                            t2->value.rational);
                    } else {
                        return strcmp(t1->value.string, t2->value.string) == 0;
                    }
                } else {
                    return FALSE;
                }
            }
            case ExprTerm: {
                return expr_is_equal(t1->expr, t2->expr);
            }
            // case ErrTerm: {
            //     return strcmp(t1->err, t2->err) == 0 ? TRUE : FALSE;
            //     break;
            // }
            default: {
                return FALSE;
            }
        }
    } else {
        return FALSE;
    }
}

Term_t term_make_number(Rational_t r) {
    struct Value v;
    v.type = RationalVal;
    v.rational = rational_copy(r);
    Term_t t = (Term_t)allocate_mem("term_make_number", NULL,
        sizeof(struct Term));
    t->type = ValTerm;
    t->value = v;
    return t;
}

Term_t term_make_string(char* str) {
    struct Value v;
    v.type = StringVal;
    v.string = str; // Don't copy strings if it isn't necessary
    Term_t t = (Term_t)allocate_mem("term_make_string", NULL,
        sizeof(struct Term));
    t->type = ValTerm;
    t->value = v;
    return t;
}

Term_t term_make_expr(Expr_t expr) {
    Term_t t = (Term_t)allocate_mem("term_make_expr", NULL,
        sizeof(struct Term));
    t->type = ExprTerm;
    t->expr = expr_copy(expr);
    return t;
}

Term_t term_make_abs(Apply_t apply, void* closure_data, size_t closure_size,
        ClosureFree_t *closure_free) {
    Term_t t = (Term_t)allocate_mem("term_make_abs", NULL,
        sizeof(struct Term));
    debug("term_make_abs - %llu %llu\n", (size_t)closure_data, (size_t)closure_free);
    t->type = AbsTerm;
    t->abs.apply = apply;
    t->abs.closure.data = closure_data;
    t->abs.closure.size = closure_size;
    t->abs.closure.closure_free = closure_free;
    return t;
}

enum TermType term_get_type(Term_t term) {
    assert(term != NULL);
    return term->type;
}

struct Value term_get_value(Term_t term) {
    assert(term != NULL);
    assert(term->type == ValTerm);

    return term->value;
}

Expr_t term_get_expr(Term_t term) {
    assert(term != NULL);
    assert(term->type == ExprTerm);

    return term->expr;
}

struct Abstraction term_get_abs(Term_t term) {
    assert(term != NULL);
    assert(term->type == AbsTerm);

    return term->abs;
}

// ErrorCode_t term_get_err(Term_t term, char** result) {
//     if (term == NULL) return Error;
//     if (term->type != ErrTerm) return Error;

//     *result = term->err;
//     return Success;
// }

void term_free(Term_t* term_ptr) {
    if (term_ptr == NULL) return;
    Term_t term = *term_ptr;
    *term_ptr = NULL;
    if (term == NULL) return;

    switch (term->type) {
        case AbsTerm : {
            // debug("term_free/abstraction - %llu, %llu, %llu\n",
                // (size_t)term, (size_t)term->abs.closure.closure_free,
                // (size_t)term->abs.closure.data);
            // debug("term_free/abstraction - %llu, %llu\n",
            //     (size_t)term, (size_t)term->abs.closure.closure_free);
            term->abs.closure.closure_free(term->abs.closure.data);
            // free_mem("free_term/closure", term->abs.closure.data);
            break;
        }
        case ValTerm: {
            // debug("term_free/value\n");
            if (term->value.type == RationalVal) {
                rational_free(term->value.rational);
                term->value.rational = NULL;
            } else {
                free_mem("free_term/string", term->value.string);
                term->value.string = NULL;
            }
            break;
        }
        case ExprTerm: {
            // debug("term_free/expression\n");
            // expr_print(term->expr);
            expr_free(&(term->expr));
            break;
        }
    }
    free_mem("term_free", term);
    *term_ptr = NULL;
}

Term_t term_copy(Term_t term) {
    debug_start("term_copy\n");
    Term_t result = (Term_t)allocate_mem("term_copy", NULL,
        sizeof(struct Term));

    result->type = term->type;
    switch (term->type) {
        case ValTerm : {
            result->value.type = term->value.type;
            if (term->value.type == StringVal) {
                result->value.string = str_cpy(term->value.string);
            } else {
                result->value.rational = rational_copy(term->value.rational);
            }
            break;
        }
        case ExprTerm : {
            result->expr = expr_copy(term->expr);
            break;
        }
        case AbsTerm : {
            result->abs.apply = term->abs.apply;
            result->abs.closure.size = term->abs.closure.size;
            result->abs.closure.closure_free = term->abs.closure.closure_free;
            if (term->abs.closure.data != NULL) {
                result->abs.closure.data =
                    allocate_mem("term_copy/closure", NULL,
                        term->abs.closure.size);
            
                memcpy(result->abs.closure.data, term->abs.closure.data,
                    term->abs.closure.size);
            } else {
                result->abs.closure.data = NULL;
            }
            break;
        }
        default: {
            break;
        }
    }

    debug_end("/term_copy\n");
    return result;
}

char* term_to_string(Term_t t) {
    if (t == NULL) return NULL;

    switch (t->type) {
        case AbsTerm: {
            char buf[100];
            sprintf(buf, "<Function with closure at %llu>",
                (size_t)t->abs.closure.data);
            return str_cpy(buf);
        }
        case ValTerm: {
            switch (t->value.type) {
                case RationalVal: {
                    return rational_to_string(t->value.rational);
                }
                case StringVal: {
                    return str_cpy(t->value.string);
                }
                default: {
                    return NULL;
                }
            }
        }
        case ExprTerm: {
            return expr_to_string(t->expr);
        }
        // case ErrTerm: {
        //     return str_cpy(t->err);
        // }
        default: {
            return NULL;
        }
    }
}

void term_print(Term_t t) {
    char* str = term_to_string(t);
    printf("%s", str);
    free_mem("term_print", str);
}

