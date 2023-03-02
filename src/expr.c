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

// Checks if two expressions are equal. Returns 1 if equal, 0 if not
uint8_t is_equal_expr(struct Expr e1, struct Expr e2) {
    // Check if their types are the same
    if (e1.type == e2.type) {
        if (e1.type == ExprAtom) {
            return strcmp(e1.symbol, e2.symbol) == 0;
        } else {
            // If both are lists, go through the lists and check the children.
            // If a child differs, none of the pointers will be null.
            // If the lists have different length, at least one of the pointers
            // will not be NULL.
            // If both lists have the same length and the same children, both
            // pointers will be NULL.
            struct Expr* list1 = e1.list;
            struct Expr* list2 = e2.list;
            uint8_t go_on = list1 != NULL && list2 != NULL;
            while (go_on) {
                if (is_equal_expr(*list1, *list2)) {
                    list1 = list1->next;
                    list2 = list2->next;
                    go_on = list1 != NULL && list2 != NULL;
                } else {
                    return 0;
                }
            }
            return list1 == NULL && list2 == NULL;
        }
    } else {
        return 0;
    }
}

// Compare two lists, and return the number of equal elements up until the first
// difference. E.g. comparing (a b c e) and (a b d e) should yield 2.
size_t match_size(struct Expr e1, struct Expr e2) {
    // Check if their types are the same
    if (e1.type == e2.type) {
        if (e1.type == ExprAtom) {
            return 0;
        } else {
            size_t match_size = 0;
            struct Expr* list1 = e1.list;
            struct Expr* list2 = e2.list;
            uint8_t go_on = list1 != NULL && list2 != NULL;
            while (go_on) {
                if (is_equal_expr(*list1, *list2)) {
                    match_size++;
                    list1 = list1->next;
                    list2 = list2->next;
                    go_on = list1 != NULL && list2 != NULL;
                } else {
                    return match_size;
                }
            }
            return match_size;
        }
    } else {
        return 0;
    }
}
