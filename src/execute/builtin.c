#include "builtin.h"

Apply_t lambda_helper1;
Apply_t lambda_helper2;
Apply_t lambda_helper3;
Term_t  execute_lambda(EnvFrame_t, void*);
ClosureFree_t lambda_free;

// let
Apply_t let_helper1;
Apply_t let_helper2;
Apply_t let_helper3;

// math operations
Apply_t binop_helper1_add;
Apply_t binop_helper1_sub;
Apply_t binop_helper1_mul;
Apply_t binop_helper1_div;
Term_t binop_helper1(enum BinOp, EnvFrame_t, Expr_t, struct Closure);
Term_t binop_helper2(EnvFrame_t, Expr_t, struct Closure);
ClosureFree_t binop_free;

Term_t make_lambda() {
    return term_make_abs(lambda_helper1, NULL, 0, lambda_free);
}

Term_t lambda_helper1(EnvFrame_t env, Expr_t name, struct Closure closure) {
    (void)closure;
    
    debug(1, "lambda_helper1 - %llu\n", (size_t)closure.closure_free);
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("lambda_helper1", NULL,
        sizeof(struct LambdaClosure));
    lambda_closure->name = expr_copy(name);
    lambda_closure->body = NULL;
    lambda_closure->value = NULL;
    Term_t result = term_make_abs(lambda_helper2, lambda_closure,
        sizeof(struct LambdaClosure), lambda_free);
    debug(-1, "/lambda_helper1\n");
    return result;
}

Term_t lambda_helper2(EnvFrame_t env, Expr_t body, struct Closure closure) {
    debug(1, "lambda_helper2\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("lambda_helper2", NULL,
        closure.size);
    memcpy(lambda_closure, closure.data, closure.size);
    lambda_closure->body = expr_copy(body);
    lambda_closure->static_env = env;
    Term_t result = term_make_abs(lambda_helper3, lambda_closure, closure.size,
        lambda_free);
    debug(-1, "/lambda_helper2\n");
    return result;
}

Term_t lambda_helper3(EnvFrame_t env, Expr_t value, struct Closure closure) {
    debug(1, "lambda_helper3\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("lambda_helper3", NULL,
        closure.size);
    memcpy(lambda_closure, closure.data, closure.size);
    lambda_closure->value = expr_copy(value);
    Term_t result = execute_lambda(env, lambda_closure);
    expr_free(&(lambda_closure->value));
    free_mem("lambda_helper3/end", lambda_closure);
    debug(-1, "/lambda_helper3\n");
    return result;
}

Term_t execute_lambda(EnvFrame_t env, void* closure_data) {
    debug(1, "execute_lambda\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure_data;
    EnvFrame_t new_frame = env_make_empty_frame(lambda_closure->static_env);

    Term_t result = eval_expr(env, lambda_closure->value);
    if (result == NULL) {
        debug(-1, "/execute_lambda\n");
        return NULL;
    }

    ErrorCode_t error_code =
        env_add_entry(new_frame, lambda_closure->name, result);
    if (error_code != Success) {
        debug(-1, "/execute_lambda\n");
        return NULL;
    }

    term_free(&result);
    result = eval_expr(new_frame, lambda_closure->body);
    if (result == NULL) {
        debug(-1, "/execute_lambda\n");
        return NULL;
    }

    env_free_frame(&new_frame);
    debug(-1, "/execute_lambda\n");
    return result;
}

void lambda_free(void* data) {
    debug(1, "lambda_free - %llu\n", (size_t)data);
    if (data == NULL) {
        debug(-1, "/lambda_free\n");
        return;
    }

    // struct LambdaClosure* lambda_closure = (struct LambdaClosure*)data;

    // debug(0, "lambda_free/name - %llu\n", (size_t)lambda_closure->name);
    // expr_free(&(lambda_closure->name));
    // debug(0, "lambda_free/body - %llu\n", (size_t)lambda_closure->body);
    // expr_free(&(lambda_closure->body));
    // debug(0, "lambda_free/value - %llu\n", (size_t)lambda_closure->value);
    // expr_free(&(lambda_closure->value));

    free_mem("lambda_free", data);
    debug(-1, "/lambda_free\n");
}

Term_t make_let() {
    return term_make_abs(let_helper1, NULL, 0, lambda_free);
}

Term_t let_helper1(EnvFrame_t env, Expr_t name, struct Closure closure) {
    debug(1, "let_helper1\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("let_helper1", NULL,
        sizeof(struct LambdaClosure));
    lambda_closure->name = expr_copy(name);
    lambda_closure->body = NULL;
    lambda_closure->value = NULL;
    Term_t result = term_make_abs(let_helper2, lambda_closure,
        sizeof(struct LambdaClosure), lambda_free);
    debug(-1, "/let_helper1\n");
    return result;
}

Term_t let_helper2(EnvFrame_t env, Expr_t value, struct Closure closure) {
    debug(1, "let_helper2\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("let_helper2", NULL,
        closure.size);
    memcpy(lambda_closure, closure.data, closure.size);
    lambda_closure->value = expr_copy(value);
    lambda_closure->static_env = env;
    Term_t result = term_make_abs(let_helper3, lambda_closure, closure.size,
        lambda_free);
    debug(-1, "/let_helper2\n");
    return result;
}

Term_t let_helper3(EnvFrame_t env, Expr_t body, struct Closure closure) {
    debug(1, "let_helper3\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("let_helper3", NULL,
        closure.size);
    memcpy(lambda_closure, closure.data, closure.size);
    lambda_closure->body = expr_copy(body);
    Term_t result = execute_lambda(env, lambda_closure);
    free_mem("let_helper3/end", lambda_closure);
    debug(-1, "/let_helper3\n");
    return result;
}

// Math functions
Term_t make_binop(enum BinOp binop) {
    Apply_t* binop_helper;
    switch (binop) {
        case ADD: {
            binop_helper = binop_helper1_add;
            break;
        }
        case SUB: {
            binop_helper = binop_helper1_sub;
            break;
        }
        case MUL: {
            binop_helper = binop_helper1_mul;
            break;
        }
        case DIV: {
            binop_helper = binop_helper1_div;
            break;
        }
        default: {
            break;
        }
    }
    return term_make_abs(binop_helper, NULL, 0, binop_free);
}

Term_t binop_helper1_add(EnvFrame_t env, Expr_t op1,
        struct Closure closure) {
    return binop_helper1(ADD, env, op1, closure);
}

Term_t binop_helper1_sub(EnvFrame_t env, Expr_t op1,
        struct Closure closure) {
    return binop_helper1(SUB, env, op1, closure);
}

Term_t binop_helper1_mul(EnvFrame_t env, Expr_t op1,
        struct Closure closure) {
    return binop_helper1(MUL, env, op1, closure);
}

Term_t binop_helper1_div(EnvFrame_t env, Expr_t op1,
        struct Closure closure) {
    return binop_helper1(DIV, env, op1, closure);
}

Term_t binop_helper1(enum BinOp binop, EnvFrame_t env, Expr_t op1,
        struct Closure closure) {
    debug(1, "binop_helper1\n");
    struct MathBinopClosure* closure_data =
        (struct MathBinopClosure*)allocate_mem(NULL, NULL,
            sizeof(struct MathBinopClosure));
    size_t closure_size = sizeof(struct MathBinopClosure);
    closure_data->binop = binop;
    closure_data->operand1 = expr_copy(op1);
    Term_t result = term_make_abs(binop_helper2, closure_data, closure_size,
        binop_free);
    debug(-1, "/binop_helper1\n");
    return result;
}

Term_t binop_helper2(EnvFrame_t env, Expr_t op2, struct Closure closure) {
    debug(1, "binop_helper2\n");
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure.data;
    Expr_t op1 = math_binop_closure->operand1;
    enum BinOp binop = math_binop_closure->binop;

    Term_t t1 = eval_expr(env, op1);
    if (t1 == NULL) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    Term_t t2 = eval_expr(env, op2);
    if (t2 == NULL) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }

    // Check if the operands are numbers
    if (t1 == NULL || t2 == NULL) {
        debug(0, "binop_helper2: at least one operand is not evaluable\n");
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    enum TermType t1type, t2type;
    ErrorCode_t error_code = term_get_type(t1, &t1type);
    if (error_code != Success) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    error_code = term_get_type(t2, &t2type);
    if (error_code != Success) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    if (t1type != ValTerm || t2type != ValTerm) {
        debug(0, "binop_helper2: at least one operand is not a value\n");
        term_free(&t1); term_free(&t2);
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    struct Value t1val, t2val;
    error_code = term_get_value(t1, &t1val);
    if (error_code != Success) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    error_code = term_get_value(t2, &t2val);
    if (error_code != Success) {
        debug(-1, "/binop_helper2\n");
        return NULL;
    }
    if (t1val.type != RationalVal || t2val.type != RationalVal) {
        debug(0, "binop_helper2: at least one operand is not a number\n");
        term_free(&t1); term_free(&t2);
        debug(-1, "/binop_helper2\n");
        return NULL;
    }

    Rational_t binop_result;
    switch (binop) {
        case ADD: {
            binop_result = rational_add(t1val.rational, t2val.rational);
            break;
        }
        case SUB: {
            binop_result = rational_sub(t1val.rational, t2val.rational);
            break;
        }
        case MUL: {
            binop_result = rational_mul(t1val.rational, t2val.rational);
            break;
        }
        case DIV: {
            binop_result = rational_div(t1val.rational, t2val.rational);
            break;
        }
    }
    Term_t result = term_make_number(binop_result);
    term_free(&t1); term_free(&t2);
    debug(-1, "/binop_helper2\n");
    return result;
}

void binop_free(void* data) {
    if (data == NULL) return;

    struct MathBinopClosure* binop_closure = (struct MathBinopClosure*)data;
    if (binop_closure->operand1 != NULL) expr_free(&(binop_closure->operand1));
    free_mem("binop_free", data);
}

ErrorCode_t add_builtin(EnvFrame_t env, char* name, Term_t term) {
    ErrorCode_t error_code;
    Expr_t expr = parse_from_str(&error_code, name);
    if (error_code != Success) {
        expr_free(&expr);
        return error_code;
    }
    env_add_entry(env, expr, term);
    term_free(&term);
    expr_free(&expr);
    return Success;
}

// Create the ground environment, with the default lookup values and the
// built-in functions
EnvFrame_t env_make_default() {
    debug(1, "env_make_default\n");
    EnvFrame_t env = env_make_empty_frame(NULL);
    if (env != NULL) {
        add_builtin(env, "lambda", make_lambda());
        add_builtin(env, "let",    make_let());
        add_builtin(env, "+",      make_binop(ADD));
        add_builtin(env, "-",      make_binop(SUB));
        add_builtin(env, "*",      make_binop(MUL));
        add_builtin(env, "/",      make_binop(DIV));
    }
    debug(1, "/env_make_default\n");
    return env;
}
