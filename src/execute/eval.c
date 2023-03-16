#include "eval.h"

ErrorCode eval_expr(struct Env env, Expr expr, struct Term* result) {
    // Read the value of this expression in the env
    struct Term t = env_lookup(env, expr);

    switch (t.type) {
        case AbsTerm: {
            // return t.abs.apply(env, expr, t.abs.closure, result);
            *result = t;
            return SUCCESS;
        }
        case ValTerm: {
            *result = t;
            return SUCCESS;
        }
        case ExprTerm: {
            // If expr is a single symbol or an empty list, return it
            if (!is_list(t.expr) || is_empty_list(t.expr)) {
                *result = t;
                return 0;
            }

            // Call eval_combination with a pointer pointing to the first
            // element of the list
            return eval_combination(env, *(t.expr + 1), result);
        }
    }
}

// Evaluate a combination (a list containing at least one element)
// Here, `expr` does not contain the opening parenthesis
ErrorCode eval_combination(struct Env env, Expr expr, struct Term* result) {
    uint8_t error_code = SUCCESS;
    // At this point, there will be no exact match, the longest matching
    // subexpression will be at least one element shorter than the expression
    // given as parameter.
    size_t matching_bytes;
    struct Term* match = find_longest_match(env, *expr, &matching_bytes);
    if (match == NULL) {
        // No match, evaluate expression from left to right. The fact that not
        // even the first element matches is not a problem, since it could be an
        // uncached list. Therefore we still need to try to evaluate it.
        error_code = eval_expr(env, expr, result);
        if (error_code != 0) {
            return error_code;
        }
    } else {
        // There is a match, take its value and continue evaluating the rest of
        // the expression from left to right
        error_code = apply(env, *match, *(expr + matching_bytes), result);
        if (error_code != 0) {
            return error_code;
        }
    }
}

ErrorCode apply(struct Env env, struct Term t, Expr e, struct Term* result) {
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
        return t.abs.apply(&env, e, result, t.abs.closure);
    }
}
