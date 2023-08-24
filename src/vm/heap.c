#include "heap.h"

#include <stdint.h>

struct Heap {
    size_t capacity;
    size_t count;
    Frame_t** frames;
};

void    _heap_grow          (Heap_t*);
void    _heap_add_operator  (Heap_t*, char*, enum PrimOp);
void    _heap_add_term      (Heap_t*, char*, Term_t*);

void _heap_grow(Heap_t* heap) {
    size_t new_size = heap->capacity < STACK_BUFFER_SIZE ?
        STACK_BUFFER_SIZE : heap->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    heap->frames = allocate_mem("_stack_grow", heap->frames,
        sizeof(Closure_t*) * new_size);
    heap->capacity = new_size;
}

void _heap_add_operator(Heap_t* heap, char* op_name, enum PrimOp op) {
    Term_t* term_op = term_make_op(op);

    // apply_op = \op. \x. \y. ((eval x) ((eval y) op))
    Term_t* term_apply_op =
        term_make_abs(str_cpy("x"),
            term_make_abs(str_cpy("y"),
                term_make_app(
                    term_make_app(
                        term_make_primval_reference("eval"),
                        term_make_primval_reference("y")),
                    term_make_app(
                        term_make_app(
                            term_make_primval_reference("eval"),
                            term_make_primval_reference("x")),
                        term_op))));

    _heap_add_term(heap, op_name, term_apply_op);
}

void _heap_add_term(Heap_t* heap, char* name, Term_t* term) {
    Frame_t* current_frame = heap_get_current_frame(heap);
    Closure_t* closure = closure_make(term, current_frame);
    Frame_t* frame = frame_make(str_cpy(name), closure, current_frame);
    heap_add(heap, frame);
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

    // Add list functions
    // _heap_add_term(heap, "$vau",    term_make_vau_raw());
    _heap_add_term(heap, "fix",     term_make_fix_raw());
    _heap_add_term(heap, "$id",     term_make_id_raw());
    _heap_add_term(heap, "eval",    term_make_eval_raw());
    _heap_add_term(heap, "pair",    term_make_pair_raw());
    _heap_add_term(heap, "leaf",    term_make_leaf_raw());
    _heap_add_term(heap, "$vau",    term_make_vau_raw());
    _heap_add_term(heap, "$lambda", term_make_lambda_raw());
    // _heap_add_term(heap, "cons",    term_make_cons_raw());
    // _heap_add_term(heap, "nil",     term_make_nil_raw());
    // _heap_add_term(heap, "head",    term_make_head_raw());
    // _heap_add_term(heap, "tail",    term_make_tail_raw());
    // _heap_add_term(heap, "true",    term_make_true_raw());
    // _heap_add_term(heap, "false",   term_make_false_raw());
    // _heap_add_term(heap, "wrap",    term_make_wrap_raw());

    // Add primops
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
    // The first NULL frame must not be serialized, hence heap->count - 1 is
    // given as argument
    serializer_write_word(serializer, heap->count - 1);
    
    for (size_t i = 1; i < heap->count; i++) {
        frame_serialize(serializer, heap, heap->frames[i]);
    }
}

Heap_t* heap_deserialize(Serializer_t* serializer) {
    Heap_t* heap = heap_make();
    // heap_count is 1 less than the final heap count, due to the first NULL
    // frame
    size_t heap_count = serializer_read_word(serializer);
    for (size_t i = 0; i < heap_count; i++) {
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

// void heap_free(Heap_t* heap) {
//     assert(heap != NULL);
    
//     // for(size_t i = 0; i < heap->capacity; i++) {
//     //     frame_free(heap->frames[i]);
//     // }
//     Frame_t* frame = heap_get_current_frame(heap);
//     while (frame != NULL) {
//         frame_free(frame);
//         heap->count--;
//         frame = heap_get_current_frame(heap);
//     }

//     free_mem("heap_free/frames", heap->frames);
//     free_mem("heap_free", heap);
// }
