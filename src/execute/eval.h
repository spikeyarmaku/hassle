#ifndef _EVAL_H_
#define _EVAL_H_

#include "config.h"
#include "global.h"

#include "execute\term.h"
#include "parse\expr.h"
#include "execute\env.h"

enum ErrorCode eval_expr         (EnvFrame_t, Expr_t, struct Term*);
enum ErrorCode eval_combination  (EnvFrame_t, Expr_t, struct Term*);
enum ErrorCode apply             (EnvFrame_t, struct Term, Expr_t, struct Term*);

#endif