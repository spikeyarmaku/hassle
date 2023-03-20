#include "builtin.h"

struct Term make_lambda() {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper1;
    t.abs.closure = allocate_mem(NULL, sizeof(struct LambdaClosure));
    return t;
}

ErrorCode _lambda_helper1(Env env, Expr name, void* closure,
        struct Term* result) {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper2;
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    lambda_closure->name = name;
    t.abs.closure = closure;
    *result = t;
    return SUCCESS;
}

ErrorCode _lambda_helper2(Env env, Expr body, void* closure,
        struct Term* result) {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _lambda_helper3;
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    lambda_closure->body = body;
    lambda_closure->static_env = env;
    t.abs.closure = closure;
    *result = t;
    return SUCCESS;
}

ErrorCode _lambda_helper3(Env env, Expr value, void* closure,
        struct Term* result) {
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure;
    ErrorCode error_code = eval_expr(env, value, result);
    if (error_code != SUCCESS) {
        return error_code;
    }
    Env new_env = lambda_closure->static_env;
    error_code = add_empty_frame(new_env);
    if (error_code != SUCCESS) {
        return error_code;
    }
    error_code = add_entry(new_env, lambda_closure->name, *result);
    if (error_code != SUCCESS) {
        return error_code;
    }

    Expr body = lambda_closure->body;
    free_mem(closure);

    return eval_expr(new_env, body, result);
}

// Math functions
struct Term make_binop(enum BinOp binop) {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _binop_helper1;
    t.abs.closure = allocate_mem(NULL, sizeof(struct MathBinopClosure));
    struct MathBinopClosure* binop_closure =
        (struct MathBinopClosure*)t.abs.closure;
    binop_closure->binop = binop;
    return t;
}

ErrorCode _binop_helper1(Env env, Expr op1, void* closure,
        struct Term* result) {
    struct Term t;
    t.type = AbsTerm;
    t.abs.apply = _binop_helper2;
    t.abs.closure = closure;
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure;
    math_binop_closure->operand1 = op1;
    *result = t;
    return SUCCESS;
}

ErrorCode _binop_helper2(Env env, Expr op2, void* closure,
        struct Term* result) {
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure;
    Expr op1 = math_binop_closure->operand1;
    enum BinOp binop = math_binop_closure->binop;
    free_mem(closure);


    struct Term t1, t2;
    ErrorCode error_code = eval_expr(env, op1, &t1);
    if (error_code != SUCCESS) {
        return error_code;
    }
    error_code = eval_expr(env, op2, &t2);
    if (error_code != SUCCESS) {
        return error_code;
    }

    // Check if the operands are numbers
    if (t1.type != ValTerm || t2.type != ValTerm) {
        return ERROR;
    }
    if (t1.value.type != RationalVal || t2.value.type != RationalVal) {
        return ERROR;
    }

    result->type = ValTerm;
    result->value.type = RationalVal;
    switch (binop) {
        case ADD: {
            result->value.rational =
                add_rational(t1.value.rational, t2.value.rational);
            break;
        }
        case SUB: {
            result->value.rational =
                sub_rational(t1.value.rational, t2.value.rational);
            break;
        }
        case MUL: {
            result->value.rational =
                mul_rational(t1.value.rational, t2.value.rational);
            break;
        }
        case DIV: {
            result->value.rational =
                div_rational(t1.value.rational, t2.value.rational);
            break;
        }
    }
    
    return SUCCESS;
}

ErrorCode _add_builtin (Env env, char* name, struct Term term) {
    Expr expr;
    ErrorCode error_code = parse_from_str(name, &expr, &(env->dict));
    if (error_code != SUCCESS) {
        return error_code;
    }
    add_entry(env, expr, term);
    return SUCCESS;
}

// Create the ground environment, with the default lookup values and the
// built-in functions
Env make_default_env() {
    Env env = make_empty_env();
    
    _add_builtin(env, "lambda", make_lambda());
    _add_builtin(env, "+", make_binop(ADD));
    _add_builtin(env, "-", make_binop(SUB));
    _add_builtin(env, "*", make_binop(MUL));
    _add_builtin(env, "/", make_binop(DIV));
    
    return env;
}
