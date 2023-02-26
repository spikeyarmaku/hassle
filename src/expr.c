#include "expr.h"

#include <stdio.h>

struct Expr* append(struct Expr* expr) {
    struct Expr* new_expr = (struct Expr*)alloc_mem(sizeof(struct Expr));
    if (expr != NULL) {
        expr->next = new_expr;
    }
    new_expr->next = NULL;
    new_expr->prev = expr;
    return new_expr;
}

struct Expr* get_head(struct Expr* expr) {
    struct Expr* current = expr;
    while (current != NULL && current->prev != NULL) {
        current = current->prev;
    }
    return current;
}

void destroy_expr(struct Expr* expr) {
    while (expr != NULL) {
        if (expr->type == ExprAtom) {
            free_mem(expr->symbol);
        } else {
            destroy_expr(expr->list);
        }

        struct Expr* next = expr->next;
        free_mem(expr);
        expr = next;
    }
}
