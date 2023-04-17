#ifndef _STACK_H_
#define _STACK_H_

#include "execute\term.h"

typedef struct Stack* Stack_t;

Stack_t stack_make_empty    ();
void    stack_push          (Stack_t, Term_t);
Term_t  stack_pop           (Stack_t);
void    stack_free          (Stack_t*);
BOOL    stack_is_empty      (Stack_t);

void    stack_print         (Stack_t);

#endif