#include "closure.h"

#include "heap.h"

struct Closure {
    Term_t* term;
    Frame_t* frame; // Not managed by Closure
};

Closure_t* closure_make(Term_t* term, Frame_t* frame) {
    Closure_t* closure = (Closure_t*)allocate_mem("closure_make", NULL,
        sizeof(struct Closure));
    closure->frame = frame;
    closure->term = term;
    return closure;
}

Term_t* closure_get_term(Closure_t* closure) {
    assert(closure != NULL);
    return closure->term;
}

Frame_t* closure_get_frame(Closure_t* closure) {
    assert(closure != NULL);
    return closure->frame;
}

Closure_t* closure_copy(Closure_t* closure) {
    Closure_t* copy = (Closure_t*)allocate_mem("closure_copy", NULL,
        sizeof(Closure_t));
    copy->frame = closure->frame;
    copy->term = term_copy(closure->term);
    return copy;
}

BOOL closure_is_update(Closure_t* closure) {
    return closure->term == NULL;
}

void closure_serialize(Serializer_t* serializer, Heap_t* heap,
    Closure_t* closure)
{
    if (closure == NULL) {
        serializer_write(serializer, 0);
        return;
    }
    
    if (closure->term == NULL) {
        serializer_write(serializer, 1);
    } else {
        serializer_write(serializer, 2);
        term_serialize(serializer, closure->term);
    }
    serializer_write_word(serializer,
        heap_get_frame_index(heap, closure->frame));
}

Closure_t* closure_deserialize(Serializer_t* serializer, Heap_t* heap)
{
    BOOL is_term_not_null = serializer_read(serializer);
    Term_t* term = NULL;
    if (is_term_not_null) {
        term = term_deserialize(serializer);
    }
    Frame_t* frame = heap_get_frame_by_index(heap,
        serializer_read_word(serializer));
    return closure_make(term, frame);
}

void closure_free(Closure_t* closure) {
    assert(closure != NULL);
    term_free(closure->term);
    free_mem("closure_free", closure);
    // NOTE decrease refcount for the frame?
}

// Free the toplevel closure, without destroying the term inside
// Useful for collecting primval terms from the stack
void closure_free_toplevel(Closure_t* closure) {
    free_mem("closure_free_toplevel", closure);
    // NOTE decrease the ref to the frame?
}
