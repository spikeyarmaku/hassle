#ifndef _EQ_STACK_H_
#define _EQ_STACK_H_

#include <stdlib.h>

#include "global.h"

struct Equation {
    struct Agent* left;
    struct Agent* right;
};

struct EqStack*     eq_stack_make   ();
void                eq_stack_push   (struct EqStack*, struct Equation);
struct Equation     eq_stack_pop    (struct EqStack*);
struct Equation     eq_stack_peek   (struct EqStack*, size_t);
void                eq_stack_free   (struct EqStack*);
size_t              eq_stack_size   (struct EqStack*);

#endif