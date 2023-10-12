#ifndef _PRIMOP_H_
#define _PRIMOP_H_

#include <stdint.h>

#include "term.h"
#include "combinators.h"

enum Primop {Add, Sub, Mul, Div, Eq};

struct Term*    primop_apply    (uint8_t, struct Term*, struct Term*);

#endif