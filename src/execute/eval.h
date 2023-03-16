#ifndef _EVAL_H_
#define _EVAL_H_

#include "config.h"
#include "global.h"

#include "execute\term.h"
#include "parse\expr.h"
#include "execute\env.h"

ErrorCode eval_expr         (struct Env*, Expr, struct Term*);
ErrorCode eval_combination  (struct Env*, Expr, struct Term*);
ErrorCode apply             (struct Env*, struct Term, Expr, struct Term*);

#endif