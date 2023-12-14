#include "stack.h"

struct Stack {
    size_t capacity;
    size_t next;
    void** elems;
};

struct ByteStack {
    size_t capacity; // in elems
    size_t next;
    size_t elem_size;
    uint8_t* data;
};

void    _stack_grow     (struct Stack*);
void    _bytestack_grow (struct ByteStack*);

void _stack_grow(struct Stack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(void*) * new_size);
        stack->capacity = new_size;
}

struct Stack* stack_make() {
    struct Stack* stack = allocate_mem("stack_make", NULL,
        sizeof(struct Stack));
    stack->capacity = 0;
    stack->elems = NULL;
    stack->next = 0;
    return stack;
}

void stack_push(struct Stack* stack, void* elem) {
    if (stack->capacity == stack->next) {
        _stack_grow(stack);
    }

    stack->elems[stack->next] = elem;
    stack->next++;
}

void* stack_pop(struct Stack* stack) {
    if (stack->next == 0) {
        return NULL;
    }
    
    stack->next--;
    void* elem = stack->elems[stack->next];

    // NOTE safety measure; can be optimized away later
    stack->elems[stack->next] = NULL;

    return elem;
}

// Return a reference to the nth element counting from the top
void* stack_peek(struct Stack* stack, size_t index) {
    if (stack->next == 0) {
        return NULL;
    }
    return stack->elems[stack->next - 1 - index];
}

// void stack_serialize(Serializer_t* serializer, Heap_t* heap, struct Stack* stack)
// {
//     serializer_write_word(serializer, stack->next);
//     for (size_t i = 0; i < stack->next; i++) {
//         closure_serialize(serializer, heap, stack->elems[i]);
//     }
// }

// struct Stack* stack_deserialize(Serializer_t* serializer, Heap_t* heap)
// {
//     Stack_t* stack = stack_make();
//     size_t count = serializer_read_word(serializer);
//     for (size_t i = 0; i < count; i++) {
//         stack_add_closure(stack, closure_deserialize(serializer, heap));
//     }
//     stack->capacity = stack->next;
//     return stack;
// }

void stack_free(struct Stack* stack) {
    assert(stack != NULL);

    free_mem("stack_free/elems", stack->elems);
    free_mem("stack_free", stack);
}

size_t stack_size(struct Stack* stack) {
    return stack->next;
}

// --- ByteStack ---

// Grow the bytestack to accomodate at least minimum_new_size amount of bytes
void _bytestack_grow(struct ByteStack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_FACTOR;
    stack->data = allocate_mem("_stack_grow", stack->data,
        sizeof(void*) * new_size * stack->elem_size);
    stack->capacity = new_size;
}

struct ByteStack* bytestack_make(size_t elem_size) {
    struct ByteStack* stack = allocate_mem("bytestack_make", NULL,
        sizeof(struct ByteStack));
    stack->capacity = 0;
    stack->data = NULL;
    stack->elem_size;
    stack->next = 0;
    return stack;
}

void bytestack_push(struct ByteStack* stack, void* elem) {
    if (stack->capacity == stack->next) {
        _stack_grow(stack);
    }

    // stack->elems[stack->next] = elem;
    memcpy(stack->data + stack->next * stack->elem_size, elem,
        stack->elem_size);
    stack->next++;
}

void* bytestack_pop(struct ByteStack* stack) {
    if (stack->next == 0) {
        return NULL;
    }
    
    stack->next--;
    void* elem = stack->data + stack->elem_size * stack->next;

    return elem;
}

// Return a reference to the nth element counting from the top
void* bytestack_peek(struct ByteStack* stack, size_t index) {
    if (stack->next == 0) {
        return NULL;
    }
    return stack->data + (stack->next - 1 - index) * stack->elem_size;
}

void bytestack_free(struct ByteStack* stack) {
    assert(stack != NULL);

    free_mem("bytestack_free/data", stack->data);
    free_mem("bytestack_free", stack);
}

size_t bytestack_size(struct ByteStack* stack) {
    return stack->next;
}