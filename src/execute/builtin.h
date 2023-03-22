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
};

struct MathBinopClosure {
    enum BinOp binop;
    Expr_t operand1;
};

// lambda
struct Term make_lambda         ();
enum ErrorCode _lambda_helper1     (EnvFrame_t, Expr_t, void*, struct Term*);
enum ErrorCode _lambda_helper2     (EnvFrame_t, Expr_t, void*, struct Term*);
enum ErrorCode _lambda_helper3     (EnvFrame_t, Expr_t, void*, struct Term*);

// math operations
struct Term make_binop          (enum BinOp);
enum ErrorCode _binop_helper1      (EnvFrame_t, Expr_t, void*, struct Term*);
enum ErrorCode _binop_helper2      (EnvFrame_t, Expr_t, void*, struct Term*);

// Create the ground environment
enum ErrorCode _add_builtin        (EnvFrame_t, char*, struct Term);
EnvFrame_t  make_default_frame  ();

#endif