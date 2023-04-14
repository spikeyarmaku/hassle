#ifndef _EVAL_H_
#define _EVAL_H_

#include "config.h"
#include "global.h"

#include "execute\term.h"
#include "parse\expr.h"
#include "execute\env.h"

enum EvalState {EvalFinished, EvalRunning};

Term_t          eval        (EnvFrame_t, Expr_t);
enum EvalState  eval_step   (EnvFrame_t);

#endif