#ifndef _STDLIB_H_
#define _STDLIB_H_

#include "execute\term.h"
#include "execute\eval.h"

#include "rational/rational.h"

enum BinOp {ADD, SUB, MUL, DIV};

// lambda
struct LambdaClosure {
    struct Env* static_env;
    Expr name;
    Expr body;
};

struct MathBinopClosure {
    enum BinOp binop;
    Expr operand1;
};

struct Term     make_lambda         ();
ErrorCode       make_lambda_helper1 (struct Env, Expr, void*, struct Term*);
ErrorCode       make_lambda_helper2 (struct Env, Expr, void*, struct Term*);
ErrorCode       make_lambda_helper3 (struct Env, Expr, void*, struct Term*);

// math operations
struct Term     make_binop          (enum BinOp);

// Create the ground environment
struct EnvFrame make_default_env    ();

#endif