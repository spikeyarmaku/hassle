#ifndef _EVAL_H_
#define _EVAL_H_

#include "config.h"
#include "global.h"

#include "term.h"
#include "expr.h"
#include "env.h"

uint8_t eval_expr   (struct Env*, struct Expr, struct Term*);
uint8_t apply       (struct Env*, struct Term, struct Expr, struct Term*);

#endif