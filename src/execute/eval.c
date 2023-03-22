#include "eval.h"

enum ErrorCode eval_expr(EnvFrame_t frame, Expr_t expr, struct Term* result) {
    // DEBUG
    char buf[1024]; print_expr(expr, frame, buf);
    debug(2, "eval_expr - %s\n", buf);
    print_env_frame(frame);
    // print_dict(env->dict, buf);
    // debug(2, "%s\n\n", buf);
    
    // Read the value of this expression in the env
    struct Term t = env_lookup_term_by_expr(frame, expr);

    switch (t.type) {
        case AbsTerm: {
            debug(2, "Abs\n");
            // return t.abs.apply(env, expr, t.abs.closure, result);
            *result = t;
            return Success;
        }
        case ValTerm: {
            debug(2, "Val\n");
            *result = t;
            return Success;
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
enum ErrorCode eval_combination(EnvFrame_t frame, Expr_t expr, struct Term* result) {
    char buf[1024]; print_expr(expr, frame, buf);
    debug(2, "eval_combination - %s\n", buf);
    // At this point, there will be no exact match, the longest matching
    // subexpression will be at least one element shorter than the expression
    // given as parameter.
    size_t matching_bytes;
    struct Term* match = find_longest_match(frame, expr, &matching_bytes);
    if (match == NULL) {
        // No match, evaluate expression from left to right. The fact that not
        // even the first element matches is not a problem, since it could be an
        // uncached list. Therefore we still need to try to evaluate it.
        Expr_t new_expr = advance_token(expr);

        enum ErrorCode Error_code = eval_expr(frame, new_expr, result);
        if (Error_code != Success) {
            return Error_code;
        }

        while (new_expr != Eos) {
            new_expr = advance_expr(new_expr);
            print_expr(new_expr, frame, buf);
            debug(2, "advance expr: - %s\n", buf);
            if (*new_expr == Eos || *new_expr == CloseParen) {
                return Success;
            } else {
                Error_code = apply(frame, *result, new_expr, result);
                if (Error_code != Success) {
                    return Error_code;
                }
            }
        }
        return Success;
    } else {
        // There is a match, take its value and continue evaluating the rest of
        // the expression from left to right
        return apply(frame, *match, expr + matching_bytes, result);
    }
}

enum ErrorCode apply(EnvFrame_t frame, struct Term t, Expr_t e, struct Term* result) {
    // char t_buf[1024]; print_term(t_buf, t, env->dict);
    // char e_buf[1024]; print_expr(e, env->dict, e_buf);
    // debug(2, "apply - %s | %s\n", t_buf, e_buf);
    debug(2, "apply\n");
    if (t.type == ValTerm) {
        return Error;
    }

    if (t.type == ExprTerm) {
        struct Term evaled;
        evaled.type = ExprTerm;
        while (evaled.type == ExprTerm) {
            uint8_t Error_code = eval_expr(frame, t.expr, &evaled);
            if (Error_code != Success) {
                return Error_code;
            }
            if (evaled.type == ExprTerm) {
                // If the evaluated expression is the same as the source
                // expression, signal an Error
                if (is_equal_expr(t.expr, evaled.expr)) {
                    return Error;
                }
            }
        }
        // free_term(evaled); // seems to do nothing?
        return apply(frame, t, e, result);
    } else {
        return t.abs.apply(frame, e, t.abs.closure, result);
    }
}
