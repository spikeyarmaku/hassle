#ifndef _STACK_H_
#define _STACK_H_

#include <stdlib.h>

#include "frame.h"
#include "closure.h"

typedef struct Stack Stack_t;

Stack_t*    stack_make              ();
void        stack_add_closure       (Stack_t*, Closure_t*);
void        stack_add_update        (Stack_t*, Frame_t*);
Closure_t*  stack_pop               (Stack_t*);
Closure_t*  stack_peek              (Stack_t*, size_t);
void        stack_free              (Stack_t*);

// DEBUG
size_t      stack_get_elem_count    (Stack_t*); // DEBUG

#endif