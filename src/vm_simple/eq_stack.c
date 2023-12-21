#include "eq_stack.h"

struct EqStack {
    size_t capacity;
    size_t count;
    struct Equation* elems;
};

void _eq_stack_grow(struct EqStack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(struct EqStack) * new_size);
    stack->capacity = new_size;
}

struct EqStack* eq_stack_make() {
    struct EqStack* stack = allocate_mem("bytestack_make", NULL,
        sizeof(struct EqStack));
    stack->capacity = 0;
    stack->count = 0;
    stack->elems = NULL;
    return stack;
}

void eq_stack_push(struct EqStack* stack, struct Equation elem) {
    if (stack->capacity == stack->count) {
        _eq_stack_grow(stack);
    }

    stack->elems[stack->count] = elem;
    stack->count++;
}

struct Equation eq_stack_pop(struct EqStack* stack) {
    if (stack->count == 0) {
        struct Equation eq;
        eq.left = NULL;
        eq.right = NULL;
        return eq;
    }
    
    stack->count--;
    return stack->elems[stack->count];
}

// Return a reference to the nth element counting from the top
struct Equation eq_stack_peek(struct EqStack* stack, size_t index) {
    return stack->elems[stack->count];
}

void eq_stack_free(struct EqStack* stack) {
    assert(stack != NULL);

    free_mem("bytestack_free", stack);
}

size_t eq_stack_size(struct EqStack* stack) {
    return stack->count;
}
