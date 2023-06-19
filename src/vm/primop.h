#ifndef _PRIMOP_H_
#define _PRIMOP_H_

#include <stdint.h>

typedef struct Closure Closure_t;

enum PrimOp {Lambda, Eval, Add, Sub, Mul, Div, Eq};

uint8_t     primop_get_arity    (enum PrimOp);
Closure_t*  primop_apply        (enum PrimOp, Closure_t**);

#endif