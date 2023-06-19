#ifndef _HEAP_H_
#define _HEAP_H_

#include "frame.h"

// Parent pointer tree
typedef struct Heap Heap_t;

Heap_t*     heap_make               ();
Heap_t*     heap_make_default       ();
Frame_t*    heap_get_current_frame  (Heap_t*);
void        heap_add                (Heap_t*, Frame_t*);
void        heap_free               (Heap_t*);

// DEBUG
size_t      heap_get_elem_count     (Heap_t*); // DEBUG
Frame_t*    heap_get_elem           (Heap_t*, size_t); // DEBUG

#endif