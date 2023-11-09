#ifndef _STACK_H_
#define _STACK_H_

#include <stdlib.h>

#include "global.h"

typedef struct Stack Stack_t;
typedef struct Heap Heap_t;

Stack_t*    stack_make      ();
void        stack_push      (Stack_t*, void*);
void*       stack_pop       (Stack_t*);
void*       stack_peek      (Stack_t*, size_t);
BOOL        stack_is_empty  (Stack_t*);
void        stack_free      (Stack_t*);

size_t      stack_count     (Stack_t*);
void*       stack_get_elem  (Stack_t*, size_t);

#endif
