#ifndef _STACK_H_
#define _STACK_H_

#include <stdlib.h>

#include "global.h"

struct Stack*       stack_make      ();
void                stack_push      (struct Stack*, void*);
void*               stack_pop       (struct Stack*);
void*               stack_peek      (struct Stack*, size_t);
void                stack_free      (struct Stack*);
size_t              stack_size      (struct Stack*);

struct ByteStack*   bytestack_make  (size_t);
void                bytestack_push  (struct ByteStack*, void*);
void*               bytestack_pop   (struct ByteStack*);
void*               bytestack_peek  (struct ByteStack*, size_t);
void                bytestack_free  (struct ByteStack*);
size_t              bytestack_size  (struct ByteStack*);

#endif