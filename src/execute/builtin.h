#ifndef _BUILTIN_H_
#define _BUILTIN_H_

#include "execute\term.h"
#include "execute\eval.h"
#include "parse\parse.h"

#include "rational/rational.h"

enum BinOp {ADD, SUB, MUL, DIV};

// lambda
struct LambdaClosure {
    EnvFrame_t static_env;
    Expr_t name;
    Expr_t body;
    Expr_t value;
};

struct MathBinopClosure {
    enum BinOp binop;
    Expr_t operand1;
};

// lambda
Term_t      make_lambda         ();
Term_t      make_let            ();
Term_t      make_binop          (enum BinOp);

// Create the ground environment
ErrorCode_t add_builtin         (EnvFrame_t, char*, Term_t);
EnvFrame_t  env_make_default    ();

#endif