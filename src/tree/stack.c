#include "stack.h"

#include <stdlib.h>

#include "config.h"

struct Stack {
    size_t capacity;
    size_t next;
    // If the first pointer in a closure is NULL, it is an update
    void** elems;
};

void _stack_grow(Stack_t*);

void _stack_grow(Stack_t* stack) {
    size_t new_size = stack->capacity < STACK_BUFFER_SIZE ?
        STACK_BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(void*) * new_size);
        stack->capacity = new_size;
}

Stack_t* stack_make() {
    Stack_t* stack = (Stack_t*)allocate_mem("stack_make", NULL,
        sizeof(Stack_t));
    stack->capacity = 0;
    stack->elems = NULL;
    stack->next = 0;
    return stack;
}

void stack_push(Stack_t* stack, void* elem) {
    if (stack->capacity == stack->next) {
        _stack_grow(stack);
    }

    stack->elems[stack->next] = elem;
    stack->next++;
}

void* stack_pop(Stack_t* stack) {
    if (stack->next == 0) {
        return NULL;
    }
    
    stack->next--;
    return stack->elems[stack->next];
}

// Return a reference to the nth element counting from the top
void* stack_peek(Stack_t* stack, size_t index) {
    if (stack->next == 0) {
        return NULL;
    }
    return stack->elems[stack->next - 1 - index];
}

BOOL stack_is_empty(Stack_t* stack) {
    if (stack == NULL) {
        return TRUE;
    }
    return stack->next == 0 ? TRUE : FALSE;
}

// Should be called only when all elements are popped
void stack_free(Stack_t* stack) {
    if (stack == NULL) {
        return;
    }
    assert(stack->next == 0);

    free_mem("stack_free/elems", stack->elems);
    free_mem("stack_free", stack);
}

size_t stack_count(Stack_t* stack) {
    return stack->next;
}

void* stack_get_elem(Stack_t* stack, size_t index) {
    return stack->elems[index];
}
