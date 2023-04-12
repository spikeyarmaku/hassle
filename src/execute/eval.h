#ifndef _EVAL_H_
#define _EVAL_H_

#include "config.h"
#include "global.h"

#include "execute\term.h"
#include "parse\expr.h"
#include "execute\env.h"

Term_t  eval_expr         (EnvFrame_t, Expr_t);
Term_t  eval_combination  (EnvFrame_t, Expr_t);
Term_t  apply             (EnvFrame_t, Term_t, Expr_t);

#endif