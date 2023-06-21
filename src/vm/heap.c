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

    // Ground frame. This is necessary for the first frame to have something to
    // reference as a parent. It is not serialized, but it is implied.
    heap_add(heap, NULL);

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

void heap_serialize(Serializer_t* serializer, Heap_t* heap) {
    printf("Serializing heap\n");
    
    // The first NULL frame must not be serialized, hence heap->count - 1 is
    // given as argument
    serializer_write_word(serializer, heap->count - 1);
    
    for (size_t i = 1; i < heap->count; i++) {
        printf("  Serializing frame #%llu\n", i);
        frame_serialize(serializer, heap, heap->frames[i]);
    }
}

Heap_t* heap_deserialize(Serializer_t* serializer) {
    Heap_t* heap = heap_make();
    // heap_count is 1 less than the final heap count, due to the first NULL
    // frame
    size_t heap_count = serializer_read_word(serializer);
    printf("Heap count: %llu\n", heap_count);
    for (size_t i = 0; i < heap_count; i++) {
        printf("  deserializing frame #%llu\n", i);
        Frame_t* frame = frame_deserialize(serializer, heap);
        heap_add(heap, frame);
    }
    heap->capacity = heap->count;
    return heap;
}

size_t heap_get_frame_index(Heap_t* heap, Frame_t* frame) {
    for (size_t i = 0; i < heap->count; i++) {
        if (heap->frames[i] == frame) {
            return i;
        }
    }
    return heap->count;
}

Frame_t* heap_get_frame_by_index(Heap_t* heap, size_t frame_index) {
    assert(frame_index < heap->count);
    return heap->frames[frame_index];
}

// size_t heap_get_elem_count(Heap_t* heap) {
//     return heap->count;
// }

// Frame_t* heap_get_elem(Heap_t* heap, size_t index) {
//     return heap->frames[index];
// }

void heap_free(Heap_t* heap) {
    assert(heap != NULL);
    
    for(size_t i = 0; i < heap->capacity; i++) {
        frame_free(heap->frames[i]);
    }
    free_mem("heap_free/frames", heap->frames);
    free_mem("heap_free", heap);
}
