#ifndef _STACK_H_
#define _STACK_H_

#include "memory.h"

struct Stack {
    struct Expr* expr;
    struct Stack* prev;
};

void            push    (struct Stack**, struct Expr*);
struct Expr*    pop     (struct Stack**);

#endif