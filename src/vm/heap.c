#include "heap.h"

#include <stdint.h>

struct Heap {
    size_t capacity;
    size_t count;
    Frame_t** frames;
};

Heap_t* heap_make() {
    Heap_t* heap = (Heap_t*)allocate_mem("heap_make", NULL, sizeof(Heap_t));
    heap->capacity = 0;
    heap->count = 0;
    heap->frames = NULL;
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

void heap_free(Heap_t* heap) {
    assert(heap != NULL);
    
    for(size_t i = 0; i < heap->capacity; i++) {
        frame_free(heap->frames[i]);
    }
    free_mem("heap_free/frames", heap->frames);
    free_mem("heap_free", heap);
}
