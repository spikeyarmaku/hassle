#include "stack.h"

struct Stack {
    size_t capacity; // How many terms can this stack hold without new allocs
    size_t pointer; // Index of the next item
    Term_t* terms;
};

Stack_t stack_make_empty() {
    Stack_t result = (Stack_t)allocate_mem("stack_make_empty", NULL,
        sizeof(struct Stack));
    result->capacity = 0;
    result->pointer = 0;
    result->terms = NULL;
    return result;
}

void stack_push(Stack_t stack, Term_t term) {
    debug_start("stack_push - %llu\n", (size_t)stack);
    if (stack->pointer == stack->capacity) {
        debug("capacity check\n");
        // Allocate more memory
        stack->terms = (Term_t*)allocate_mem("stack_push", stack->terms,
            sizeof(Term_t) * (stack->capacity + STACK_BUFFER_SIZE));
        stack->capacity += STACK_BUFFER_SIZE;
        debug("capacity check done\n");
    }
    stack->terms[stack->pointer] = term_copy(term);
    stack->pointer++;
    debug_end("/stack_push\n");
}

Term_t stack_pop(Stack_t stack) {
    if (stack->pointer == 0) {
        return NULL;
    }
    stack->pointer--;
    return stack->terms[stack->pointer];
}

void stack_free(Stack_t* stack_ptr) {
    if (stack_ptr == NULL) return;
    struct Stack* stack = *stack_ptr;

    // destroy all elements
    Term_t temp = stack_pop(stack);
    while (temp != NULL) {
        term_free(&temp);
        temp = stack_pop(stack);
    }

    // destroy the list
    free_mem("stack_free/terms", stack->terms);

    free_mem("stack_free", stack);

    *stack_ptr = NULL;
}

BOOL stack_is_empty(Stack_t stack) {
    return stack->pointer == 0;
}
