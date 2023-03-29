#include "eval.h"

enum ErrorCode eval_expr(EnvFrame_t frame, Expr_t expr, struct Term* result) {
    // DEBUG
    debug(1, "eval_expr\n");
    // debug(0, "\nExpr: "); expr_print(expr); debug(0, "\nFrame:");
    // env_print_frame(frame); debug(0, "\n");
    
    // Read the value of this expression in the env
    struct Term t = env_lookup_term(frame, expr);

    switch (t.type) {
        case AbsTerm: {
            debug(0, "Abs\n");
            // return t.abs.apply(env, expr, t.abs.closure, result);
            *result = t;
            debug(-1, "/eval_expr\n");
            return Success;
        }
        case ValTerm: {
            debug(0, "Val\n");
            *result = t;
            debug(-1, "/eval_expr\n");
            return Success;
        }
        case ExprTerm: {
            debug(0, "Expr\n");
            // If expr is a single symbol or an empty list, return it
            if (!expr_is_list(t.expr) || expr_is_empty_list(t.expr)) {
                *result = t;
                debug(-1, "/eval_expr/symbol_or_empty_list\n");
                return 0;
            }

            #ifdef MEMOIZE_SUB_EXPRS
            // TODO
            #endif
            debug(-1, "/eval_expr\n");
            return eval_combination(frame, t.expr, result);
        }
        // Control shouldn't reach this point
        default: {
            return Error;
        }
    }
}

// Evaluate a combination (a list containing at least one element)
// Here, `expr` does not contain the opening parenthesis
enum ErrorCode eval_combination(EnvFrame_t frame, Expr_t expr,
        struct Term* result) {
    debug(1, "eval_combination\n");
    expr_print(expr); printf("\n");
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

        enum ErrorCode error_code = eval_expr(frame, new_expr, result);
        if (error_code != Success) {
            debug(-1, "eval_combination\n");
            return error_code;
        }

        new_expr = expr_get_next(new_expr);
        while (new_expr != NULL) {
            // printf("New expr: "); expr_print(new_expr); printf("\n");
            error_code = apply(frame, *result, new_expr, result);
            if (error_code != Success) {
                debug(-1, "eval_combination\n");
                return error_code;
            }
            new_expr = expr_get_next(new_expr);
        }
        debug(-1, "eval_combination\n");
        return Success;
    } else {
        // There is a match, take its value and continue evaluating the rest of
        // the expression from left to right
        debug(-1, "eval_combination\n");
        return apply(frame, *match, expr, result);
    }
}

enum ErrorCode apply(EnvFrame_t frame, struct Term t, Expr_t e,
        struct Term* result) {
    // char t_buf[1024]; print_term(t_buf, t, env->dict);
    // char e_buf[1024]; print_expr(e, env->dict, e_buf);
    // debug(2, "apply - %s | %s\n", t_buf, e_buf);
    debug(1, "apply\n");
    if (t.type == ValTerm) {
        debug(-1, "/apply\n");
        return Error;
    }

    if (t.type == ExprTerm) {
        struct Term evaled;
        evaled.type = ExprTerm;
        while (evaled.type == ExprTerm) {
            uint8_t error_code = eval_expr(frame, t.expr, &evaled);
            if (error_code != Success) {
                debug(-1, "/apply\n");
                return error_code;
            }
            if (evaled.type == ExprTerm) {
                // If the evaluated expression is the same as the source
                // expression, signal an Error
                if (expr_is_equal(t.expr, evaled.expr)) {
                    debug(-1, "/apply\n");
                    return Error;
                }
            }
        }
        // term_free(evaled); // seems to do nothing?
        debug(-1, "/apply\n");
        return apply(frame, t, e, result);
    } else {
        debug(-1, "/apply\n");
        return t.abs.apply(frame, e, t.abs.closure, result);
    }
}
