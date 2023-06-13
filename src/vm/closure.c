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
    return closure->term;
}

Frame_t* closure_get_frame(Closure_t* closure) {
    return closure->frame;
}

void closure_free(Closure_t* closure) {
    assert(closure != NULL);
    term_free(closure->term);
    free_mem("closure_free", closure);
    // TODO decrease refcount for the frame?
}
