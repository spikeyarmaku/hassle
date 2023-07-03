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

Term_t* _term_read_rational     (char*);
Term_t* _term_read_string       (char*);
Term_t* _term_read_symbol       (char*);
Term_t* _term_primval_from_expr (Expr_t*);

// Try to parse the string as a rational. If unsuccessful, return NULL
// TODO Make it work with hex
Term_t* _term_read_rational(char* str) {
    // valid numbers: (# - any number of digits, ? - sign, . - dec separator)
    // # .# #. #.# ?# ?.# ?#. ?#.#
    // starts with #: # #. #.#
    // starts with .: .#
    // starts with ?: ?# ?.# ?#. ?#.#

    size_t first_char_index = 0;
    // If the first character is a sign, it's ok, don't count it as the first
    if (str[0] == '+' || str[0] == '-') {
        first_char_index = 1;
    }

    size_t span = strspn(str + first_char_index, "0123456789.,_");
    size_t len = strlen(str + first_char_index);

    if (span == len && span != 0) {
        return
            term_make_primval(primval_make_rational(rational_from_string(str)));
    } else {
        return NULL;
    }
}

// Check if the string is a valid string (beings and ends with "). If
// unsuccessful, return NULL
Term_t* _term_read_string(char* str) {
    if (str[0] != '\"') {
        return NULL;
    }

    size_t i = 0;
    while (str[i] != 0) {
        i++;
    }
    if (str[i - 1] != '\"') {
        return NULL;
    }

    char* str_content = (char*)allocate_mem("_term_read_string", NULL,
        sizeof(char) * (i - 2));
    strncpy(str_content, str + 1, i - 2);
    str_content[i - 2] = 0;

    free_mem("_term_read_string", str);

    return term_make_primval(primval_make_string(str_content));
}

// Read the string as a symbol
Term_t* _term_read_symbol(char* str) {
    return term_make_primval(primval_make_symbol(str));
}

Term_t* _term_primval_from_expr(Expr_t* expr) {
    // expr is an atom
    char* symbol = str_cpy(expr_get_symbol(expr));
    // Check if it is a literal (rational or string)
    Term_t* term = _term_read_rational(symbol);
    if (term == NULL) {
        term = _term_read_string(symbol);
        if (term == NULL) {
            term = _term_read_symbol(symbol);
        }
    }
    return term;
}

// TODO While converting a term to expr, collect variable names into a list,
// replace the variables in the resulting term with their index in the list,
// and return the list along with the term
Term_t* term_from_expr(Expr_t* expr) {
    if (!expr_is_list(expr)) {
        // expr is an atom
        char* symbol = str_cpy(expr_get_symbol(expr));
        // Check if it is a literal (rational or string)
        Term_t* term = _term_read_rational(symbol);
        if (term == NULL) {
            term = _term_read_string(symbol);
            if (term == NULL) {
                term = _term_read_symbol(symbol);
            }
        }
        return term;
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
            result = term_make_app(term1, term2);
        }
        
        return result;
    }
}

// Turn an expression into a scott-encoded binary tree
// https://stackoverflow.com/questions/30655680/how-do-you-represent-nested-types-using-the-scott-encoding
// As explained in the following blog post, preprocessing avoids trivializing
// the equational theory of the language, while still supporting metaprogramming
// http://fexpr.blogspot.com/2013/07/explicit-evaluation.html
Term_t* term_from_expr_encoded(Expr_t* expr) {
    if (!expr_is_list(expr)) {
        return term_make_app(term_make_leaf(), _term_primval_from_expr(expr));
    } else {
        // expr is a list

        // Construct the list from its end
        // (+ 1 2) -> ((+ 1) 2) -> ((cons ((cons +) 1)) 2)
        Term_t* result = NULL;
        for (size_t i = 0; i < expr_get_child_count(expr); i++) {
            Expr_t* new_expr = expr_get_child(expr, i);
            Term_t* new_term = NULL;
            new_term = term_from_expr_encoded(new_expr);
            if (result == NULL) {
                result = new_term;
            } else {
                result =
                    term_make_app(
                        term_make_app(term_make_pair(), result), new_term);
            }
        }
        return result;
    }
}

Term_t* term_make_primval(PrimVal_t* val) {
    Term_t* term = (Term_t*)allocate_mem("term_make_primval", NULL,
        sizeof(struct Term));
    term->type = PrimValTerm;
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

Term_t* term_make_app(Term_t* term1, Term_t* term2) {
    Term_t* term = (Term_t*)allocate_mem("term_make_app", NULL,
        sizeof(struct Term));
    term->type = AppTerm;
    term->app.term1 = term1;
    term->app.term2 = term2;
    return term;
}

Term_t* term_make_op(enum PrimOp op) {
    Term_t* term = (Term_t*)allocate_mem("term_make_op", NULL,
        sizeof(Term_t));
    term->type = OpTerm;
    term->op = op;
    return term;
}

enum TermType term_get_type(Term_t* term) {
    assert(term != NULL);
    return term->type;
}

PrimVal_t* term_get_primval(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == PrimValTerm);
    return term->primval;
}

char* term_get_abs_var(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == AbsTerm);
    return term->abs.var;
}

Term_t* term_get_abs_body(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == AbsTerm);
    return term->abs.app_body;
}

Term_t* term_get_app_term1(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == AppTerm);
        // term_get_type(term) == StrictAppTerm);
    return term->app.term1;
}

Term_t* term_get_app_term2(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == AppTerm);
        // term_get_type(term) == StrictAppTerm);
    return term->app.term2;
}

enum PrimOp term_get_op(Term_t* term) {
    assert(term != NULL);
    assert(term_get_type(term) == OpTerm);
    return term->op;
}

BOOL term_is_self_evaluating(Term_t* term) {
    if (term->type == AppTerm) {
        return FALSE;
    }
    if (term->type == PrimValTerm) {
        if (primval_get_type(term_get_primval(term)) == SymbolValue) {
            return FALSE;
        }
    }
    return TRUE;
}

Term_t* term_copy(Term_t* term) {
    assert(term != NULL);
    switch(term->type) {
        case PrimValTerm:
            return term_make_primval(primval_copy(term->primval));
        case AbsTerm:
            return term_make_abs(
                str_cpy(term->abs.var), term_copy(term->abs.app_body));
        case AppTerm:
            return term_make_app(
                term_copy(term->app.term1), term_copy(term->app.term2));
        // case StrictAppTerm:
        //     return term_make_strict_app(
        //         term_copy(term->app.term1), term_copy(term->app.term2));
        case OpTerm:
            return term_make_op(term->op);
        case WorldTerm:
            // TODO
            return NULL;
        default: assert(FALSE); return NULL;
    }
}

// void term_free(Term_t* term) {
//     assert(term != NULL);
//     switch (term->type) {
//         case PrimValTerm: {
//             primval_free(term->primval);
//             break;
//         }
//         case AbsTerm:
//             free_mem("term_free/abs/var", term->abs.var);
//             term_free(term->abs.app_body);
//             break;
//         case AppTerm:
//         // case StrictAppTerm:
//             term_free(term->app.term1);
//             term_free(term->app.term2);
//             break;
//         case OpTerm:
//         case WorldTerm:
//             break;
//         default: {
//             assert(FALSE);
//         }
//     }
//     free_mem("term_free", term);
// }

// // Free the toplevel term, without destroying the child elements
// // Useful for deconstructing terms from the VM's control register, and putting
// // back one of the constituents
// void term_free_toplevel(Term_t* term) {
//     free_mem("term_free_toplevel", term);
// }

void term_serialize(Serializer_t* serializer, Term_t* term) {
    assert(term != NULL);
    serializer_write(serializer, (uint8_t)term->type);
    switch (term->type) {
        case PrimValTerm:
            primval_serialize(serializer, term->primval);
            break;
        case AbsTerm:
            serializer_write_string(serializer, term->abs.var);
            term_serialize(serializer, term->abs.app_body);
            break;
        case AppTerm:
        // case StrictAppTerm:
            term_serialize(serializer, term->app.term1);
            term_serialize(serializer, term->app.term2);
            break;
        case OpTerm:
            serializer_write(serializer, (uint8_t)term->op);
            break;
        case WorldTerm:
            // TODO
            break;
        default:
            assert(FALSE);
            break;
    }
}

Term_t* term_deserialize(Serializer_t* serializer) {
    assert(serializer != NULL);
    enum TermType type = (enum TermType)serializer_read(serializer);
    switch (type) {
        case PrimValTerm: {
            PrimVal_t* primval = primval_deserialize(serializer);
            return term_make_primval(primval);
        }
        case AbsTerm: {
            char* name = serializer_read_string(serializer);
            Term_t* term = term_deserialize(serializer);
            return term_make_abs(name, term);
        }
        case AppTerm: {
            Term_t* term1 = term_deserialize(serializer);
            Term_t* term2 = term_deserialize(serializer);
            return term_make_app(term1, term2);
        }
        // case StrictAppTerm: {
        //     Term_t* term1 = term_deserialize(serializer);
        //     Term_t* term2 = term_deserialize(serializer);
        //     return term_make_strict_app(term1, term2);
        // }
        case OpTerm: {
            enum PrimOp op = (enum PrimOp)serializer_read(serializer);
            return term_make_op(op);
        }
        case WorldTerm: {
            // TODO
            break;
        }
        default: {
            assert(FALSE);
            break;
        }
    }
    // TODO
    return NULL;
}

Term_t* term_make_primval_symbol(char* name) {
    return term_make_primval(primval_make_symbol(str_cpy(name)));
}

void term_print(Term_t* term) {
    assert(term != NULL);
    switch (term->type) {
        case PrimValTerm:
            printf("<Primval ");
            primval_print(term->primval);
            printf(">");
            break;
        case AbsTerm:
            printf("<Abs %s ", term->abs.var);
            term_print(term->abs.app_body);
            printf(">");
            break;
        case AppTerm:
            printf("<App ");
            term_print(term->app.term1);
            printf(" ");
            term_print(term->app.term2);
            printf(">");
            break;
        // case StrictAppTerm:
        //     printf("<StrictApp ");
        //     term_print(term->app.term1);
        //     printf(" ");
        //     term_print(term->app.term2);
        //     printf(">");
        //     break;
        case OpTerm:
            switch(term->op) {
                // case Vau:   printf("<OpVau>");  break;
                // case Eval:  printf("<OpEval>"); break;
                case Add:   printf("<OpAdd>");  break;
                case Sub:   printf("<OpSub>");  break;
                case Mul:   printf("<OpMul>");  break;
                case Div:   printf("<OpDiv>");  break;
                case Eq:    printf("<OpEq>");   break;
            }
            break;
        case WorldTerm:
            printf("<World>");
            break;
        default:
            assert(FALSE);
    }
}

Term_t* term_make_cons() {
    return term_make_primval_symbol("cons");
}

Term_t* term_make_nil() {
    return term_make_primval_symbol("nil");
}

Term_t* term_make_head() {
    return term_make_primval_symbol("head");
}

Term_t* term_make_tail() {
    return term_make_primval_symbol("tail");
}

Term_t* term_make_true() {
    return term_make_primval_symbol("true");
}

Term_t* term_make_false() {
    return term_make_primval_symbol("false");
}

Term_t* term_make_pair() {
    return term_make_primval_symbol("pair");
}
Term_t* term_make_leaf() {
    return term_make_primval_symbol("leaf");
}

// id = \x. x
Term_t* term_make_vau_id_raw() {
    return
        term_make_abs("x", term_make_primval_symbol("x"));
}

// fix = \f. (\x. f (x x)) (\x. f (x x))
Term_t* term_make_fix_raw() {
    return
        term_make_abs("f",
            term_make_app(
                term_make_abs("x",
                    term_make_app(term_make_primval_symbol("f"),
                        term_make_app(term_make_primval_symbol("x"),
                            term_make_primval_symbol("x")))),
                term_make_abs("x",
                    term_make_app(term_make_primval_symbol("f"),
                        term_make_app(term_make_primval_symbol("x"),
                            term_make_primval_symbol("x"))))));
}


// Scott-encoded cons: \x. \xs. \n. \c. c x xs
// (x = head, y = tail, n = what-to-do-if-nil, c = what-to-do-if-pair)
Term_t* term_make_cons_raw() {
    return
        term_make_abs(str_cpy("x"),
            term_make_abs(str_cpy("xs"),
                term_make_abs(str_cpy("n"),
                    term_make_abs(str_cpy("c"),
                        term_make_app(
                            term_make_app(
                                term_make_primval(
                                    primval_make_symbol(str_cpy("c"))),
                                term_make_primval(
                                    primval_make_symbol(str_cpy("x")))),
                            term_make_primval(
                                primval_make_symbol(str_cpy("xs"))))))));
}

// Scott-encoded nil: \n. \c. n, which is the same as `true` (\x. \y. x)
Term_t* term_make_nil_raw() {
    return term_make_true();
}

// Scott-encoded head: \list. list undef (\x. \xs. x)
Term_t* term_make_head_raw() {
    return
        term_make_abs(str_cpy("list"),
            term_make_app(
                term_make_app(
                    term_make_primval(primval_make_symbol(str_cpy("list"))),
                    term_make_primval(primval_make_symbol(str_cpy("u")))),
                term_make_true()));
}

// Scott-encoded tail: \list. list undef (\x. \xs. xs)
Term_t* term_make_tail_raw() {
    return
        term_make_abs(str_cpy("list"),
            term_make_app(
                term_make_app(
                    term_make_primval(primval_make_symbol(str_cpy("list"))),
                    term_make_primval(primval_make_symbol(str_cpy("u")))),
                term_make_false()));
}


// true = first = \x. \y. x
Term_t* term_make_true_raw() {
    return
        term_make_abs(str_cpy("x"),
            term_make_abs(str_cpy("y"),
                term_make_primval(
                    primval_make_symbol(str_cpy("x")))));
}

// false = second = \x. \y. y
Term_t* term_make_false_raw() {
    return
        term_make_abs(str_cpy("x"),
            term_make_abs(str_cpy("y"),
                term_make_primval(
                    primval_make_symbol(str_cpy("y")))));
}

Term_t* term_make_pair_raw() {
    return
        term_make_abs("x",
            term_make_abs("y",
                term_make_abs("p",
                    term_make_abs("l",
                        term_make_app(
                            term_make_app(
                                term_make_primval_symbol("p"),
                                term_make_primval_symbol("x")),
                            term_make_primval_symbol("y"))))));
}

Term_t* term_make_leaf_raw() {
    return
        term_make_abs("x",
            term_make_abs("p",
                term_make_abs("l",
                    term_make_app(
                        term_make_primval_symbol("l"),
                        term_make_primval_symbol("x")))));
}

Term_t* term_make_eval_raw() {
    return
        term_make_app(term_make_primval_symbol("fix"),
    // _eval = \eval. \tree. (tree (\x. \y. (eval x) y)) (\x.x)
            term_make_abs("eval",
                term_make_abs("tree",
                    term_make_app(
                        term_make_app(
                            term_make_primval_symbol("tree"),
                            term_make_abs("x",
                                term_make_abs("y",
                                    term_make_app(
                                        term_make_app(
                                            term_make_primval_symbol("eval"),
                                            term_make_primval_symbol("x")),
                                        term_make_primval_symbol("y"))))),
                        term_make_primval_symbol("$id")))));
}

// \f. \x. f (eval x)
Term_t* term_make_wrap_raw() {
    return
        term_make_abs("f",
            term_make_abs("x",
                term_make_app(
                    term_make_primval_symbol("f"),
                    term_make_app(
                        term_make_primval_symbol("eval"),
                        term_make_primval_symbol("x")))));
}

// \f. \x. (wrap f) x
Term_t* term_make_id_raw() {
    return
        term_make_app(
            term_make_primval_symbol("wrap"),
            term_make_primval_symbol("$id"));
}

// Take a term, containing only lazy apps and primvals, and emit a list-encoded
// term
Term_t* term_encode_as_list(Term_t* term) {
    assert(term != NULL);

    if (term_get_type(term) == PrimValTerm) {
        return
            term_make_app(
                term_make_app(term_make_cons(), term),
                term_make_nil());
    } else {
        // term must be app
        assert(term_get_type(term) == AppTerm);
        Term_t* result =
            term_make_app(
                term_make_app(
                    term_make_cons(),
                    term_encode_as_list(term_get_app_term1(term))),
                term_encode_as_list(term_get_app_term2(term)));
        // term_free_toplevel(term);
        return result;
    }
}
