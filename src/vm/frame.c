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

void frame_free(Frame_t* frame) {
    assert(frame != NULL);

    free_mem("frame_free/name", frame->name);
    closure_free(frame->value);
}
