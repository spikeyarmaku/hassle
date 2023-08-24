#ifndef _PRIMOP_H_
#define _PRIMOP_H_

#include <stdint.h>

typedef struct Closure Closure_t;

enum PrimOp {Vau, Add, Sub, Mul, Div, Eq, MakeApp, WithEnv};

uint8_t     primop_get_arity    (enum PrimOp);
Closure_t*  primop_apply        (enum PrimOp, Closure_t**);

#endif