#include "stack.h"

#include <stdlib.h>

#include "closure.h"

struct Stack {
    size_t capacity;
    size_t next;
    // If the first pointer in a closure is NULL, it is an update
    Closure_t** elems;
};

void _stack_grow(Stack_t*);

void _stack_grow(Stack_t* stack) {
    size_t new_size = stack->capacity < STACK_BUFFER_SIZE ?
        STACK_BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(Closure_t*) * new_size);
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

void stack_add_closure(Stack_t* stack, Closure_t* closure) {
    if (stack->capacity == stack->next) {
        _stack_grow(stack);
    }

    stack->elems[stack->next] = closure;
    stack->next++;
}

void stack_add_update(Stack_t* stack, Frame_t* frame) {
    stack_add_closure(stack, closure_make(NULL, frame));
}

Closure_t* stack_pop(Stack_t* stack) {
    if (stack->next == 0) {
        return NULL;
    }
    
    stack->next--;
    Closure_t* closure = stack->elems[stack->next];

    // NOTE safety measure; can be optimized away later
    stack->elems[stack->next] = NULL;

    return closure;
}

// Return a reference to the nth element counting from the top
Closure_t* stack_peek(Stack_t* stack, size_t index) {
    if (stack->next == 0) {
        return NULL;
    }
    return stack->elems[stack->next - 1 - index];
}

void stack_serialize(Serializer_t* serializer, Heap_t* heap, Stack_t* stack)
{
    serializer_write_word(serializer, stack->next);
    for (size_t i = 0; i < stack->next; i++) {
        closure_serialize(serializer, heap, stack->elems[i]);
    }
}

Stack_t* stack_deserialize(Serializer_t* serializer, Heap_t* heap)
{
    Stack_t* stack = stack_make();
    size_t count = serializer_read_word(serializer);
    for (size_t i = 0; i < count; i++) {
        stack_add_closure(stack, closure_deserialize(serializer, heap));
    }
    stack->capacity = stack->next;
    return stack;
}

// void stack_free(Stack_t* stack) {
//     assert(stack != NULL);

//     for (size_t i = 0; i < stack->capacity; i++) {
//         closure_free(stack->elems[i]);
//     }
//     free_mem("stack_free/elems", stack->elems);
//     free_mem("stack_free", stack);
// }

size_t stack_get_elem_count(Stack_t* stack) {
    return stack->next;
}

