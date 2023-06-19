#include "heap.h"

#include <stdint.h>

struct Heap {
    size_t capacity;
    size_t count;
    Frame_t** frames;
};

void    _heap_grow          (Heap_t*);
void    _heap_add_operator  (Heap_t*, char*, enum PrimOp);

void _heap_grow(Heap_t* heap) {
    size_t new_size = heap->capacity < STACK_BUFFER_SIZE ?
        STACK_BUFFER_SIZE : heap->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    heap->frames = allocate_mem("_stack_grow", heap->frames,
        sizeof(Closure_t*) * new_size);
}

void _heap_add_operator(Heap_t* heap, char* op_name, enum PrimOp op) {
    Term_t* term_op = term_make_op(op);
    Frame_t* current_frame = heap_get_current_frame(heap);
    Closure_t* closure_op = closure_make(term_op, current_frame);
    Frame_t* frame_op = frame_make(op_name, closure_op, current_frame);
    heap_add(heap, frame_op);
}

Heap_t* heap_make() {
    Heap_t* heap = (Heap_t*)allocate_mem("heap_make", NULL, sizeof(Heap_t));
    heap->capacity = 0;
    heap->count = 0;
    heap->frames = NULL;
    return heap;
}

Heap_t* heap_make_default() {
    Heap_t* heap = heap_make();

    // Add operators
    _heap_add_operator(heap, "+", Add);
    _heap_add_operator(heap, "-", Sub);
    _heap_add_operator(heap, "*", Mul);
    _heap_add_operator(heap, "/", Div);

    return heap;
}

Frame_t* heap_get_current_frame(Heap_t* heap) {
    if (heap == NULL) {
        return NULL;
    }
    if (heap->count == 0) {
        return NULL;
    }
    return heap->frames[heap->count - 1];
}

void heap_add(Heap_t* heap, Frame_t* frame) {
    if (heap->capacity == heap->count) {
        _heap_grow(heap);
    }
    heap->frames[heap->count] = frame;
    heap->count++;
}

size_t heap_get_elem_count(Heap_t* heap) {
    return heap->count;
}

Frame_t* heap_get_elem(Heap_t* heap, size_t index) {
    return heap->frames[index];
}

void heap_free(Heap_t* heap) {
    assert(heap != NULL);
    
    for(size_t i = 0; i < heap->capacity; i++) {
        frame_free(heap->frames[i]);
    }
    free_mem("heap_free/frames", heap->frames);
    free_mem("heap_free", heap);
}