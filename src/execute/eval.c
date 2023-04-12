#include "eval.h"

Term_t eval_expr(EnvFrame_t frame, Expr_t expr) {
    // DEBUG
    debug(1, "eval_expr\n"); // expr_print(expr); debug(0, "\n");
    
    // Read the value of this expression in the env
    Term_t term = env_lookup_term(frame, expr);
    if (term == NULL) {
        debug(-1, "/eval_expr\n");
        return NULL;
    }

    enum TermType term_type;
    ErrorCode_t error_code = term_get_type(term, &term_type);
    if (error_code != Success) {
        debug(-1, "/eval_expr\n");
        return NULL;
    }
    switch (term_type) {
        case AbsTerm: {
            debug(0, "[Abs]\n");
            debug(-1, "/eval_expr\n");
            return term;
        }
        case ValTerm: {
            debug(0, "[Val]\n");
            debug(-1, "/eval_expr\n");
            return term;
        }
        case ExprTerm: {
            debug(0, "[Expr]\n");
            Expr_t term_expr;
            error_code = term_get_expr(term, &term_expr);
            if (error_code != Success) {
                debug(-1, "/eval_expr\n");
                return NULL;
            }
            // If expr is a single symbol or an empty list, return it
            if (!expr_is_list(term_expr) || expr_is_empty_list(term_expr)) {
                debug(-1, "/eval_expr/symbol_or_empty_list\n");
                return term;
            }

            #ifdef MEMOIZE_SUB_EXPRS
            // TODO
            #endif
            // term_expr = expr_copy(term_expr);
            Term_t result = eval_combination(frame, term_expr);
            term_free(&term);
            debug(-1, "/eval_expr\n");
            return result;
        }
        // Control shouldn't reach this point
        default: {
            return NULL;
        }
    }
}

// Evaluate a combination (a list containing at least one element)
// Here, `expr` does not contain the opening parenthesis
Term_t eval_combination(EnvFrame_t frame, Expr_t expr) {
    debug(1, "eval_combination\n");
    // expr_print(expr); printf("\n");
    
    // At this point, there will be no exact match, the longest matching
    // subexpression will be at least one element shorter than the expression
    // given as parameter.
    // TODO fix (currently it doesnt report a match if the first element is
    // there in the env, but as a single symbol, such as `lambda`)
    struct Term* match = env_find_longest_match(frame, expr, NULL);
    if (match == NULL) {
        // No match, evaluate expression from left to right. The fact that not
        // even the first element matches is not a problem, since it could be an
        // uncached list. Therefore we still need to try to evaluate it.
        Expr_t new_expr = expr_get_list(expr);

        Term_t new_result = eval_expr(frame, new_expr);
        if (new_result == NULL) {
            debug(-1, "/eval_combination\n");
            return NULL;
        }

        new_expr = expr_get_next(new_expr);
        while (new_expr != NULL) {
            Term_t temp_result = apply(frame, new_result, new_expr);
            if (temp_result == NULL) {
                debug(-1, "/eval_combination/error\n");
                return NULL;
            }
            term_free(&new_result);
            new_result = temp_result;
            new_expr = expr_get_next(new_expr);
        }
        debug(-1, "/eval_combination/end\n");
        return new_result;
    } else {
        // There is a match, take its value and continue evaluating the rest of
        // the expression from left to right
        Term_t result = apply(frame, match, expr);
        term_free(&match);
        debug(-1, "/eval_combination\n");
        return result;
    }
}

// Given a term and an expression, apply the term to the expression
Term_t apply(EnvFrame_t frame, Term_t term, Expr_t expr) {
    debug(1, "apply\n");
    
    // Get the term's type
    enum TermType term_type;
    ErrorCode_t error_code = term_get_type(term, &term_type);
    if (error_code != Success) {
        debug(-1, "/apply\n");
        return NULL;
    }

    // If the term is a value, return an error
    switch (term_type) {
        case ValTerm: {
            debug(-1, "/apply\n");
            return NULL;
        }
        case AbsTerm: {
            debug(0, "AbsTerm\n");
            struct Abstraction term_abs;
            ErrorCode_t error_code = term_get_abs(term, &term_abs);
            if (error_code != Success) {
                debug(-1, "/apply\n");
                return NULL;
            }
            Term_t result = term_abs.apply(frame, expr, term_abs.closure);
            debug(-1, "/apply\n");
            return result;
        }
        case ExprTerm: {
            debug(0, "ExprTerm\n");
            Expr_t prev_expr = NULL;
            Expr_t curr_expr = NULL;
            Term_t evaled;
            while (term_type == ExprTerm) {
                ErrorCode_t error_code = term_get_expr(term, &curr_expr);
                if (error_code != Success) {
                    debug(-1, "/apply\n");
                    return NULL;
                }
                evaled = eval_expr(frame, curr_expr);
                error_code = term_get_type(evaled, &term_type);
                if (error_code != Success) {
                    debug(-1, "/apply\n");
                    return NULL;
                }
                if (expr_is_equal(prev_expr, curr_expr)) return NULL;
                
                prev_expr = curr_expr;
            }
            Term_t result = apply(frame, evaled, expr);
            term_free(&evaled);
            return result;
        }
        default: {
            return NULL;
        }
    }
}
