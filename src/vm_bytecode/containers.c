#include "containers.h"

#include <stdio.h>

struct EqStack {
    size_t capacity;
    size_t count;
    struct Equation* elems;
};

struct AgentHeap {
    size_t capacity;
    size_t count;
    struct Agent** elems;
};

struct ByteArray {
    size_t capacity;
    size_t count;
    uint8_t* elems;
};

void _eq_stack_grow(struct EqStack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(struct Equation) * new_size);
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
    stack->count--;
    return stack->elems[stack->count];
}

// Return a reference to the nth element counting from the top
struct Equation eq_stack_peek(struct EqStack* stack, size_t index) {
    return stack->elems[index];
}

void eq_stack_free(struct EqStack* stack) {
    assert(stack != NULL);

    free_mem("bytestack_free", stack);
}

size_t eq_stack_size(struct EqStack* stack) {
    return stack->count;
}

// ---------------------

struct AgentHeap* agent_heap_make() {
    struct AgentHeap* heap =
        allocate_mem("agent_heap_make", NULL, sizeof(struct AgentHeap));
    heap->capacity = 0;
    heap->count = 0;
    heap->elems = NULL;
    return heap;
}

struct AgentHeap* agent_heap_make_fixed(size_t count) {
    struct AgentHeap* heap =
        allocate_mem("agent_heap_make_fixed", NULL, sizeof(struct AgentHeap));
    heap->capacity = 0;
    heap->count = 0;
    heap->elems = allocate_mem("agent_heap_make_fixed/elems", NULL,
        sizeof(struct Agent*) * count);
    return heap;
}

void agent_heap_add(struct AgentHeap* heap, struct Agent* elem) {
    while (heap->count + 1 > heap->capacity) {
        heap->capacity = heap->capacity == 0 ?
            BUFFER_SIZE : heap->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
        heap->elems = allocate_mem("ptrpool_add_elem", heap->elems,
            heap->capacity * sizeof(struct Agent*));
    }

    agent_heap_add_unsafe(heap, elem);
}

void agent_heap_add_unsafe(struct AgentHeap* heap, struct Agent* elem) {
    heap->elems[heap->count] = elem;
    heap->count++;
}

size_t agent_heap_get_count(struct AgentHeap* heap) {
    return heap->count;
}

void agent_heap_set(struct AgentHeap* heap, size_t index, struct Agent* elem) {
    if (heap->capacity > index) {
        heap->elems[index] = elem;
    }
}

struct Agent* agent_heap_get(struct AgentHeap* heap, size_t index) {
    if (heap->capacity > index) {
        return heap->elems[index];
    } else {
        return NULL;
    }
}

void agent_heap_free(struct AgentHeap* heap) {
    for (size_t i = 0; i < heap->count; i++) {
        agent_free(heap->elems[i]);
    }
    free_mem("agent_heap_free/elems", heap->elems);
    free_mem("agent_heap_free", heap);
}

// ------------------------------

void _byte_array_grow(struct ByteArray* byte_array) {
    size_t new_size = byte_array->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : byte_array->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    byte_array->elems = allocate_mem("_byte_array_grow", byte_array->elems,
        sizeof(uint8_t) * new_size);
    byte_array->capacity = new_size;
}

struct ByteArray* byte_array_make() {
    struct ByteArray* byte_array = allocate_mem("byte_array_make", NULL,
        sizeof(struct ByteArray));
    byte_array->capacity = 0;
    byte_array->count = 0;
    byte_array->elems = NULL;
    return byte_array;
}

void byte_array_add_byte(struct ByteArray* byte_array, uint8_t elem) {
    if (byte_array->count == byte_array->capacity) {
        _byte_array_grow(byte_array);
    }

    // printf("ByteArray[%llu] = %d\n", byte_array->count, elem);
    byte_array->elems[byte_array->count] = elem;
    byte_array->count++;
}

void byte_array_add_word(struct ByteArray* byte_array, size_t elem,
    uint8_t word_size)
{
    while (byte_array->capacity < byte_array->count + word_size) {
        _byte_array_grow(byte_array);
    }
    word_to_bytes(elem, byte_array->elems + byte_array->count, word_size);

    // for (size_t i = byte_array->count; i < byte_array->count + word_size; i++) {
    //     printf("ByteArray[%llu] = %d (%llu)\n", i, byte_array->elems[i], elem);
    // }

    byte_array->count += word_size;
}

uint8_t*  byte_array_get_bytes(struct ByteArray* byte_array) {
    uint8_t* bytes = allocate_mem("byte_array_get_bytes", NULL,
        sizeof(uint8_t) * byte_array->count);
    for (size_t i = 0; i < byte_array->count; i++) {
        bytes[i] = byte_array->elems[i];
    }
    free_mem("byte_array_get_bytes/elems", byte_array->elems);
    free_mem("byte_array_get_bytes", byte_array);
    return bytes;
}
