#include "eval.h"

ErrorCode eval_expr(Env env, Expr expr, struct Term* result) {
    // DEBUG
    // char buf[1024]; print_expr(expr, env->dict, buf);
    // debug(2, "eval_expr(%s) - ", buf);
    // print_dict(env->dict, buf);
    // debug(2, "%s\n\n", buf);
    
    // Read the value of this expression in the env
    struct Term t = env_lookup(env, expr);

    switch (t.type) {
        case AbsTerm: {
            debug(2, "Abs\n");
            // return t.abs.apply(env, expr, t.abs.closure, result);
            *result = t;
            return SUCCESS;
        }
        case ValTerm: {
            debug(2, "Val\n");
            *result = t;
            return SUCCESS;
        }
        case ExprTerm: {
            debug(2, "Expr\n");
            // If expr is a single symbol or an empty list, return it
            if (!is_list(t.expr) || is_empty_list(t.expr)) {
                *result = t;
                return 0;
            }

            #ifdef MEMOIZE_SUB_EXPRS
            // TODO
            #endif
            return eval_combination(env, t.expr, result);
        }
        // Control shouldn't reach this point
        default: {
            return ERROR;
        }
    }
}

// Evaluate a combination (a list containing at least one element)
// Here, `expr` does not contain the opening parenthesis
ErrorCode eval_combination(Env env, Expr expr, struct Term* result) {
    debug(2, "eval_combination - ");
    // At this point, there will be no exact match, the longest matching
    // subexpression will be at least one element shorter than the expression
    // given as parameter.
    size_t matching_bytes;
    struct Term* match = find_longest_match(env, expr, &matching_bytes);
    if (match == NULL) {
        debug(2, "No match\n");
        // No match, evaluate expression from left to right. The fact that not
        // even the first element matches is not a problem, since it could be an
        // uncached list. Therefore we still need to try to evaluate it.
        Expr new_expr = advance_token(expr);

        ErrorCode error_code = eval_expr(env, new_expr, result);
        if (error_code != SUCCESS) {
            return error_code;
        }

        while (new_expr != Eos) {
            new_expr = advance_expr(new_expr);
            if (*new_expr == Eos || *new_expr == CloseParen) {
                return SUCCESS;
            } else {
                error_code = apply(env, *result, new_expr, result);
                if (error_code != SUCCESS) {
                    return error_code;
                }
            }
        }
        return SUCCESS;
    } else {
        debug(2, "Match\n");
        // There is a match, take its value and continue evaluating the rest of
        // the expression from left to right
        return apply(env, *match, expr + matching_bytes, result);
    }
}

ErrorCode apply(Env env, struct Term t, Expr e, struct Term* result) {
    debug(2, "apply - ");
    if (t.type == ValTerm) {
        return ERROR;
    }

    if (t.type == ExprTerm) {
        struct Term evaled;
        evaled.type = ExprTerm;
        while (evaled.type == ExprTerm) {
            uint8_t error_code = eval_expr(env, t.expr, &evaled);
            if (error_code != SUCCESS) {
                return error_code;
            }
            if (evaled.type == ExprTerm) {
                // If the evaluated expression is the same as the source
                // expression, signal an error
                if (is_equal_expr(t.expr, evaled.expr)) {
                    return ERROR;
                }
            }            
        }
        return apply(env, t, e, result);
    } else {
        debug(2, "abstraction - calling apply\n");
        return t.abs.apply(env, e, t.abs.closure, result);
    }
}
