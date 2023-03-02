#include "eval.h"

uint8_t eval_expr(struct Env* env, struct Expr expr, struct Term* result) {
    // Read the value of this expression in the env
    struct Term t = env_lookup(env, expr);

    switch (t.type) {
        case AbsTerm: {
            *result = t.apply(env, expr);
            return 0;
        }
        case ValTerm: {
            *result = t;
            return 0;
        }
        case ExprTerm: {
            // If expr is a single symbol or an empty list, return it
            if (t.expr.type == ExprAtom || t.expr.list == NULL) {
                *result = t;
                return 0;
            }
            // Expr is a combination
            // The list has one element, evaluate it
            if (t.expr.list->next == NULL) {
                return eval_expr(env, *(t.expr.list), result);
            } else {
                // Find the longest match in env
                size_t match_size;
                struct Term* match =
                    find_longest_match(env, t.expr, &match_size);
                struct Term first_elem;
                struct Expr* expr_ptr = t.expr.list;
                uint8_t error_code = 0;
                if (match == NULL) {
                    // No match, evaluate expression from left to right
                    // The fact that not even the first element matches is not
                    // a problem, since it could be a list, which is not cached.
                    // Therefore we still need to try to evaluate it.
                    error_code = eval_expr(env, *(t.expr.list), &first_elem);
                    if (error_code != 0) {
                        return error_code;
                    }
                    expr_ptr = expr_ptr->next;
                } else {
                    // There is a match, take its value and continue evaluating
                    // the rest of the expression from left to right
                    first_elem = *match;
                    for (size_t i = 0; i < match_size; i++) {
                        expr_ptr = expr_ptr->next;
                    }
                }
                while (expr_ptr != NULL) {
                    error_code = apply(env, first_elem, *expr_ptr, &first_elem);
                    if (error_code != 0) {
                        return error_code;
                    }
                    expr_ptr = expr_ptr->next;
                }
                *result = first_elem;
                return 0;
            }
        }
    }
}

uint8_t apply(struct Env* env, struct Term t, struct Expr e,
        struct Term* result) {
    if (t.type == ValTerm) {
        return 1;
    }

    if (t.type == ExprTerm) {
        struct Term evaled;
        uint8_t error_code = eval_expr(env, t.expr, &evaled);
        if (evaled.type == ExprTerm) {
            // If the evaluated expression is the same as the source expression
            if (is_equal_expr(t.expr, evaled.expr)) {
                return 1;
            } else {
                return apply(env, evaled, e, result);
            }
        } else {
            return apply(env, t, e, result);
        }
    } else {
        *result = t.apply(env, e);
        return 0;
    }
}
