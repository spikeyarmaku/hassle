#ifndef _HEAP_H_
#define _HEAP_H_

#include "frame.h"
#include "serialize\serialize.h"

// Parent pointer tree
typedef struct Heap Heap_t;

Heap_t*     heap_make               ();
Heap_t*     heap_make_default       ();
Frame_t*    heap_get_current_frame  (Heap_t*);
void        heap_add                (Heap_t*, Frame_t*);
void        heap_serialize          (Serializer_t*, Heap_t*);
Heap_t*     heap_deserialize        (Serializer_t*);
size_t      heap_get_frame_index    (Heap_t*, Frame_t*);
Frame_t*    heap_get_frame_by_index (Heap_t*, size_t);
// void        heap_free               (Heap_t*);

// DEBUG
// size_t      heap_get_elem_count     (Heap_t*); // DEBUG
// Frame_t*    heap_get_elem           (Heap_t*, size_t); // DEBUG

#endif