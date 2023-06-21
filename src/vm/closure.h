#ifndef _CLOSURE_H_
#define _CLOSURE_H_

// #include "frame.h"
#include "term.h"
#include "serialize\serialize.h"

typedef struct Frame Frame_t;
typedef struct Closure Closure_t;
typedef struct Heap Heap_t;

Closure_t*  closure_make            (Term_t*, Frame_t*);
Term_t*     closure_get_term        (Closure_t*);
Frame_t*    closure_get_frame       (Closure_t*);
Closure_t*  closure_copy            (Closure_t*);
BOOL        closure_is_update       (Closure_t*);
void        closure_serialize       (Serializer_t*, Heap_t*, Closure_t*);
Closure_t*  closure_deserialize     (Serializer_t*, Heap_t*);
void        closure_free            (Closure_t*);
void        closure_free_toplevel   (Closure_t*);

#endif