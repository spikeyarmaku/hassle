#include "term.h"

// This is a lambda term, extended with symbols and built-in values (numbers and
// strings)
struct Term {
    enum TermType type;
    union {
        // Primval
        PrimVal_t* primval;

        // Abstraction
        struct {
            char* var;
            Term_t* app_body;
        } abs;

        // Application
        struct {
            Term_t* term1;
            Term_t* term2;
        } app;

        enum PrimOp op;
    };
};

// TODO While converting a term to expr, collect variable names into a list,
// replace the variables in the resulting term with their index in the list,
// and return the list along with the term
//
// Turn an expression into symbols and lazy apps
Term_t* term_from_expr(Expr_t* expr) {
    if (!expr_is_list(expr)) {
        // expr is an atom
        char* symbol = str_cpy(expr_get_symbol(expr));
        PrimVal_t* symbol_val = primval_make_symbol(symbol);
        return term_make_primval(symbol_val);
    } else {
        // expr is a list

        // empty list is illegal
        assert(!expr_is_empty_list(expr));

        // a list with one element is just the element itself
        if (expr_get_child_count(expr) == 1) {
            return term_from_expr(expr_get_child(expr, 0));
        }

        // a list with more than one elements is converted into left-associative
        // successive pairs - e.g. (a b c d) -> (((a b) c) d)
        Term_t* result = NULL;
        Term_t* term1;
        Term_t* term2;
        for (size_t i = 0; i < expr_get_child_count(expr) - 1; i++) {
            if (result == NULL) {
                term1 = term_from_expr(expr_get_child(expr, i));
            } else {
                term1 = result;
            }
            term2 = term_from_expr(expr_get_child(expr, i + 1));
            result = term_make_lazy_app(term1, term2);
        }
        
        return result;
    }
}

Term_t* term_make_primval(PrimVal_t* val) {
    Term_t* term = (Term_t*)allocate_mem("term_make_primval", NULL,
        sizeof(struct Term));
    term->type = PrimvalTerm;
    term->primval = val;
    return term;
}

Term_t* term_make_abs(char* var_name, Term_t* body) {
    Term_t* term = (Term_t*)allocate_mem("term_make_abs", NULL,
        sizeof(struct Term));
    term->type = AbsTerm;
    term->abs.var = var_name;
    term->abs.app_body = body;
    return term;
}

Term_t* term_make_syntax(char* var_name, Term_t* body) {
    Term_t* term = (Term_t*)allocate_mem("term_make_abs", NULL,
        sizeof(struct Term));
    term->type = AbsTerm;
    term->abs.var = var_name;
    term->abs.app_body = body;
    return term;
}

Term_t* term_make_lazy_app(Term_t* term1, Term_t* term2) {
    Term_t* term = (Term_t*)allocate_mem("term_make_lazy_app", NULL,
        sizeof(struct Term));
    term->type = LazyAppTerm;
    term->app.term1 = term1;
    term->app.term2 = term2;
    return term;
}

Term_t* term_make_strict_app(Term_t* term1, Term_t* term2) {
    Term_t* term = (Term_t*)allocate_mem("term_make_lazy_app", NULL,
        sizeof(struct Term));
    term->type = StrictAppTerm;
    term->app.term1 = term1;
    term->app.term2 = term2;
    return term;
}

enum TermType term_get_type(Term_t* term) {
    return term->type;
}

PrimVal_t* term_get_primval(Term_t* term) {
    assert(term_get_type(term) == PrimvalTerm);
    return term->primval;
}

char* term_get_abs_var(Term_t* term) {
    assert(term_get_type(term) == AbsTerm ||
        term_get_type(term) == SyntaxTerm);
    return term->abs.var;
}

Term_t* term_get_abs_body(Term_t* term) {
    assert(term_get_type(term) == AbsTerm ||
        term_get_type(term) == SyntaxTerm);
    return term->abs.app_body;
}

Term_t* term_get_app_term1(Term_t* term) {
    assert(term_get_type(term) == LazyAppTerm ||
        term_get_type(term) == StrictAppTerm);
    return term->app.term1;
}

Term_t* term_get_app_term2(Term_t* term) {
    assert(term_get_type(term) == LazyAppTerm ||
        term_get_type(term) == StrictAppTerm);
    return term->app.term2;
}

void term_free(Term_t* term) {
    assert(term != NULL);
    switch (term->type) {
        case PrimvalTerm: {
            primval_free(term->primval);
            break;
        }
        case AbsTerm:
        case SyntaxTerm:
            free_mem("term_free/abs/var", term->abs.var);
            term_free(term->abs.app_body);
            break;
        case LazyAppTerm:
        case StrictAppTerm:
            term_free(term->app.term1);
            term_free(term->app.term2);
            break;
        case OpTerm:
        case WorldTerm:
            break;
        default: {
            assert(FALSE);
        }
    }
    free_mem("term_free", term);
}

void term_print(Term_t* term) {
    switch (term->type) {
        case PrimvalTerm: {
            printf("<Val ");
            primval_print(term->primval);
            printf(">");
            break;
        }
        case AbsTerm: {
            printf("<Abs %s ", term->abs.var);
            term_print(term->abs.app_body);
            printf(">");
            break;
        }
        case SyntaxTerm: {
            printf("<Syntax %s ", term->abs.var);
            term_print(term->abs.app_body);
            printf(">");
            break;
        }
        case LazyAppTerm: {
            printf("<LazyApp ");
            term_print(term->app.term1);
            printf(" ");
            term_print(term->app.term2);
            printf(">");
            break;
        }
        case StrictAppTerm: {
            printf("<StrictApp ");
            term_print(term->app.term1);
            printf(" ");
            term_print(term->app.term2);
            printf(">");
            break;
        }
        case OpTerm: {
            switch(term->op) {
                case Lambda:    printf("<OpLambda>");   break;
                case Eval:      printf("<OpEval>");     break;
                case Add:       printf("<OpAdd>");      break;
                case Sub:       printf("<OpSub>");      break;
                case Mul:       printf("<OpMul>");      break;
                case Div:       printf("<OpDiv>");      break;
                case Eq:        printf("<OpEq>");       break;
            }
            break;
        }
        case WorldTerm: {
            printf("<World>");
            break;
        }
        default: {
            assert(FALSE);
        }
    }
}
