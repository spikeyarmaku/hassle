#ifndef _PRIMOP_H_
#define _PRIMOP_H_

#include <stdint.h>

#include "tree.h"
#include "combinators.h"

enum Primop {Add, Sub, Mul, Div, Eq};

struct Tree*    primop_apply    (uint8_t, struct Tree*, struct Tree*);

#endif