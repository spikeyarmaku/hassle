#ifndef _BUILTIN_H_
#define _BUILTIN_H_

#include "execute\term.h"
#include "execute\eval.h"
#include "parse\parse.h"

#include "rational/rational.h"

enum BinOp {ADD, SUB, MUL, DIV};

// lambda
struct LambdaClosure {
    Env static_env;
    Expr name;
    Expr body;
};

struct MathBinopClosure {
    enum BinOp binop;
    Expr operand1;
};

// lambda
struct Term     make_lambda     ();
ErrorCode       _lambda_helper1 (Env, Expr, void*, struct Term*);
ErrorCode       _lambda_helper2 (Env, Expr, void*, struct Term*);
ErrorCode       _lambda_helper3 (Env, Expr, void*, struct Term*);

// math operations
struct Term     make_binop      (enum BinOp);
ErrorCode       _binop_helper1  (Env, Expr, void*, struct Term*);
ErrorCode       _binop_helper2  (Env, Expr, void*, struct Term*);

// Create the ground environment
ErrorCode       _add_builtin        (Env, char*, struct Term);
Env             make_default_env    ();

#endif