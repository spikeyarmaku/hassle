#include "containers.h"

struct EqStack {
    size_t capacity;
    size_t count;
    struct Equation* elems;
};

struct AgentHeap {
    uint8_t capacity;
    uint8_t count;
    struct Agent** elems;
};

void _eq_stack_grow(struct EqStack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_stack_grow", stack->elems,
        sizeof(struct EqStack) * new_size);
    stack->capacity = new_size;
}

struct EqStack* eq_stack_make(size_t elem_size) {
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
    return stack->elems[stack->count];
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
