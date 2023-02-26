#include "stack.h"

#include <stdio.h>

void push(struct Stack** stack, struct Expr* expr) {
    if (expr != NULL) {
        struct Stack* new_stack = (struct Stack*)alloc_mem(sizeof(struct Stack));
        new_stack->expr = expr;
        new_stack->prev = *stack;
        *stack = new_stack;
    }
}

struct Expr* pop(struct Stack** stack) {
    struct Stack* old_stack = *stack;
    if (old_stack != NULL) {
        *stack = (*stack)->prev;
        struct Expr* expr = old_stack->expr;
        free_mem(old_stack);
        return expr;
    } else {
        return NULL;
    }
}
