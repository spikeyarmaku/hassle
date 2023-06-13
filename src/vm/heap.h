#ifndef _HEAP_H_
#define _HEAP_H_

#include "frame.h"

// Parent pointer tree
typedef struct Heap Heap_t;

Heap_t*     heap_make               ();
Frame_t*    heap_get_current_frame  (Heap_t*);
void        heap_free               (Heap_t*);

#endif