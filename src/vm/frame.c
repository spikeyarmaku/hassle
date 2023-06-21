#include "frame.h"

#include "vm\closure.h"
#include "heap.h"

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

void frame_serialize(Serializer_t* serializer, Heap_t* heap, Frame_t* frame) {
    if (frame == NULL) {
        // Don't serialize the ground frame
        return;
    }

    serializer_write_string(serializer, frame->name);
    closure_serialize(serializer, heap, frame->value);
    size_t parent_index = heap_get_frame_index(heap, frame->parent);
    printf("%s %llu", frame->name, parent_index);
    serializer_write_word(serializer, parent_index);
}

Frame_t* frame_deserialize(Serializer_t* serializer, Heap_t* heap) {
    char* name = serializer_read_string(serializer);
    printf("%s ", name);
    Closure_t* closure = closure_deserialize(serializer, heap);
    size_t parent_index = serializer_read_word(serializer);

    printf("%llu\n", parent_index);
    return frame_make(name, closure,
        heap_get_frame_by_index(heap, parent_index));
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


