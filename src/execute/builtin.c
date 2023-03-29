#include "builtin.h"

struct Term make_lambda() {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper1;
    t.abs.closure =
        allocate_mem("make_lambda", NULL, sizeof(struct LambdaClosure));
    return t;
}

enum ErrorCode _lambda_helper1(EnvFrame_t env, Expr_t name, void* closure,
        struct Term* result) {
    debug(1, "_lambda_helper1\n");
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper2;
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    lambda_closure->name = name;
    t.abs.closure = closure;
    *result = t;
    debug(-1, "/_lambda_helper1\n");
    return Success;
}

enum ErrorCode _lambda_helper2(EnvFrame_t env, Expr_t body, void* closure,
        struct Term* result) {
    debug(1, "_lambda_helper2\n");
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper3;
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    lambda_closure->body = body;
    lambda_closure->static_env = env;
    t.abs.closure = closure;
    *result = t;
    debug(-1, "/_lambda_helper2\n");
    return Success;
}

enum ErrorCode _lambda_helper3(EnvFrame_t env, Expr_t value, void* closure,
        struct Term* result) {
    debug(1, "_lambda_helper3\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    enum ErrorCode Error_code = eval_expr(env, value, result);
    if (Error_code != Success) {
        debug(-1, "/_lambda_helper3\n");
        return Error_code;
    }
    
    EnvFrame_t new_frame = env_make_empty_frame(lambda_closure->static_env);
    Error_code = env_add_entry(new_frame, lambda_closure->name, *result);
    if (Error_code != Success) {
        debug(-1, "/_lambda_helper3\n");
        return Error_code;
    }
    
    Error_code = eval_expr(new_frame, lambda_closure->body, result);
    if (Error_code != Success) {
        debug(-1, "/_lambda_helper3\n");
        return Error_code;
    }

    // free_mem("_lambda_helper3", closure);
    env_free_frame(&new_frame); // TODO free env (currently the program crashes at this line)
    debug(-1, "/_lambda_helper3\n");
    return Success;
}

// Math functions
struct Term make_binop(enum BinOp binop) {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _binop_helper1;
    t.abs.closure =
        allocate_mem("make_binop", NULL, sizeof(struct MathBinopClosure));
    struct MathBinopClosure* binop_closure =
        (struct MathBinopClosure*)t.abs.closure;
    binop_closure->binop = binop;
    return t;
}

enum ErrorCode _binop_helper1(EnvFrame_t env, Expr_t op1, void* closure,
        struct Term* result) {
    debug(1, "_binop_helper1\n");
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _binop_helper2;
    t.abs.closure = closure;
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure;
    math_binop_closure->operand1 = op1;
    *result = t;
    debug(-1, "/_binop_helper1\n");
    return Success;
}

enum ErrorCode _binop_helper2(EnvFrame_t env, Expr_t op2, void* closure,
        struct Term* result) {
    debug(1, "_binop_helper2\n");
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure;
    Expr_t op1 = math_binop_closure->operand1;
    enum BinOp binop = math_binop_closure->binop;

    struct Term t1, t2;
    enum ErrorCode Error_code = eval_expr(env, op1, &t1);
    if (Error_code != Success) {
        debug(-1, "/_binop_helper2\n");
        return Error_code;
    }
    Error_code = eval_expr(env, op2, &t2);
    if (Error_code != Success) {
        debug(-1, "/_binop_helper2\n");
        return Error_code;
    }

    // Check if the operands are numbers
    if (t1.type != ValTerm || t2.type != ValTerm) {
        debug(0, "_binop_helper2: at least one operand is not a value\n");
        // term_free(t1); term_free(t2);
        debug(-1, "/_binop_helper2\n");
        return Error;
    }
    if (t1.value.type != RationalVal || t2.value.type != RationalVal) {
        debug(0, "_binop_helper2: at least one operand is not a number\n");
        // term_free(t1); term_free(t2);
        debug(-1, "/_binop_helper2\n");
        return Error;
    }

    result->type = ValTerm;
    result->value.type = RationalVal;
    switch (binop) {
        case ADD: {
            result->value.rational =
                rational_add(t1.value.rational, t2.value.rational);
            break;
        }
        case SUB: {
            result->value.rational =
                rational_sub(t1.value.rational, t2.value.rational);
            break;
        }
        case MUL: {
            result->value.rational =
                rational_mul(t1.value.rational, t2.value.rational);
            break;
        }
        case DIV: {
            result->value.rational =
                rational_div(t1.value.rational, t2.value.rational);
            break;
        }
    }
    // free_mem("_binop_helper2/closure", closure);
    // term_free(t1); term_free(t2);
    debug(-1, "/_binop_helper2\n");
    return Success;
}

enum ErrorCode _add_builtin (EnvFrame_t env, char* name, struct Term term) {
    ErrorCode_t error_code;
    Expr_t expr = parse_from_str(&error_code, name);
    if (error_code != Success) {
        expr_free(&expr);
        return error_code;
    }
    env_add_entry(env, expr, term);
    expr_free(&expr);
    return Success;
}

// Create the ground environment, with the default lookup values and the
// built-in functions
EnvFrame_t env_make_default() {
    EnvFrame_t env = env_make_empty_frame(NULL);
    if (env != NULL) {
        _add_builtin(env, "lambda", make_lambda());
        _add_builtin(env, "+",      make_binop(ADD));
        _add_builtin(env, "-",      make_binop(SUB));
        _add_builtin(env, "*",      make_binop(MUL));
        _add_builtin(env, "/",      make_binop(DIV));
    }
    return env;
}
