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
    stack->terms[stack->pointer] = term;
    stack->pointer++;
    debug_end("/stack_push\n");
}

Term_t stack_pop(Stack_t stack) {
    debug_start("stack_pop - %llu\n", (size_t)stack);
    if (stack->pointer == 0) {
        debug_end("/stack_push\n");
        return NULL;
    }
    // term_free(&(stack->terms[stack->pointer]));
    stack->pointer--;
    Term_t result = stack->terms[stack->pointer];
    stack->terms[stack->pointer] = NULL;
    debug_end("/stack_pop\n");
    return result;
}

void stack_free(Stack_t* stack_ptr) {
    if (stack_ptr == NULL) return;
    struct Stack* stack = *stack_ptr;

    // destroy all elements
    Term_t temp;
    do {
        temp = stack_pop(stack);
        if (temp != NULL) {
            term_free(&temp);
        }
    } while (temp != NULL);

    // destroy the list
    free_mem("stack_free/terms", stack->terms);

    free_mem("stack_free", stack);

    *stack_ptr = NULL;
}

BOOL stack_is_empty(Stack_t stack) {
    return stack->pointer == 0;
}

void stack_print(Stack_t stack) {
    debug_start("stack_print - %llu elems\n", stack->pointer);
    for (size_t i = stack->pointer; i > 0; i--) {
        debug("%llu. elem: ", i - 1);
        term_print(stack->terms[i - 1]);
        debug("\n");
    }
    debug_end("/stack_print\n");
}
