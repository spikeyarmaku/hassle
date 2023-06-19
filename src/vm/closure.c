#include "closure.h"

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
