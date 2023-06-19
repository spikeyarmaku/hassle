#include "frame.h"

#include "vm\closure.h"

struct Frame {
    char* name;
    Closure_t* value;
    Frame_t* parent;
};

Frame_t* frame_make(char* name, Closure_t* value, Frame_t* parent) {
    Frame_t* frame = (Frame_t*)allocate_mem("frame_make", NULL,
        sizeof(struct Frame));
    frame->name = name;
    frame->value = value;
    frame->parent = parent;
    return frame;
}

void frame_update(Frame_t* frame, Closure_t* new_value) {
    closure_free(frame->value);
    frame->value = new_value;
}

// Find the value assigned to this variable name in the environment and return a
// copy of the value. If there isn't any, return the symbol itself.
Closure_t* frame_lookup(Frame_t* frame, char* var_name) {
    // Climb the spaghetti stack and look for the variable name
    Frame_t* current_frame = frame;
    while (current_frame != NULL) {
        if (strcmp(current_frame->name, var_name) == 0) {
            return closure_copy(current_frame->value);
        }
        current_frame = current_frame->parent;
    }

    // Haven't found a value assigned to this variable, make a new symbol
    return closure_make(term_make_primval(primval_make_symbol(var_name)),
        frame);
}

void frame_free(Frame_t* frame) {
    assert(frame != NULL);

    free_mem("frame_free/name", frame->name);
    closure_free(frame->value);
}

char* frame_get_name(Frame_t* frame) {
    return frame->name;
}

Closure_t* frame_get_value(Frame_t* frame) {
    return frame->value;
}

Frame_t* frame_get_parent(Frame_t* frame) {
    return frame->parent;
}


