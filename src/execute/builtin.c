#include "builtin.h"

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
Term_t      make_eq             ();

// Create the ground environment
ErrorCode_t add_builtin         (EnvFrame_t, char*, Term_t);

Apply_t lambda_helper1;
Apply_t lambda_helper2;
Apply_t lambda_helper3;
Term_t  execute_lambda(EnvFrame_t, void*);
ClosureFree_t lambda_free;
ClosureCopy_t lambda_copy;

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
Term_t execute_binop(EnvFrame_t, Expr_t, struct Closure);
ClosureFree_t binop_free;
ClosureCopy_t binop_copy;

// Term_t make_lambda() {
//     return term_make_abs(lambda_helper1, NULL, 0, lambda_free);
// }

Term_t make_lambda() {
    return term_make_abs(lambda_helper1, NULL, 0, lambda_free, lambda_copy);
}

Term_t lambda_helper1(EnvFrame_t env, Expr_t name, struct Closure closure) {
    (void)closure;

    debug_start("lambda_helper1\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("lambda_helper1", NULL,
        sizeof(struct LambdaClosure));
    lambda_closure->name = name;
    lambda_closure->body = NULL;
    lambda_closure->value = NULL;
    lambda_closure->static_env = NULL;
    Term_t result = term_make_abs(lambda_helper2, lambda_closure,
        sizeof(struct LambdaClosure), lambda_free, lambda_copy);
    debug_end("/lambda_helper1\n");
    return result;
}

Term_t lambda_helper2(EnvFrame_t env, Expr_t body, struct Closure closure) {
    debug_start("lambda_helper2\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("lambda_helper2", NULL,
        closure.size);
    lambda_closure->name =((struct LambdaClosure*)closure.data)->name;
    ((struct LambdaClosure*)closure.data)->name = NULL;
    lambda_closure->body = body;
    lambda_closure->value = NULL;
    lambda_closure->static_env = env;
    Term_t result = term_make_abs(lambda_helper3, lambda_closure, closure.size,
        lambda_free, lambda_copy);
    debug_end("/lambda_helper2\n");
    return result;
}

Term_t lambda_helper3(EnvFrame_t env, Expr_t value, struct Closure closure) {
    debug_start("lambda_helper3\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure.data;
    lambda_closure->value = value;
    Term_t result = execute_lambda(env, lambda_closure);
    debug_end("/lambda_helper3\n");
    return result;
}

Term_t execute_lambda(EnvFrame_t env, void* closure_data) {
    debug_start("execute_lambda\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure_data;

    Term_t result = eval(env, lambda_closure->value);
    if (result == NULL) {
        debug_end("/execute_lambda\n");
        return NULL;
    }
    lambda_closure->value = NULL;

    EnvFrame_t new_frame = env_make_empty_frame(lambda_closure->static_env);
    ErrorCode_t error_code =
        env_add_entry(new_frame, lambda_closure->name, result);
    if (error_code != Success) {
        debug_end("/execute_lambda\n");
        return NULL;
    }
    lambda_closure->name = NULL;

    result = eval(new_frame, lambda_closure->body);
    if (result == NULL) {
        debug_end("/execute_lambda\n");
        return NULL;
    }
    lambda_closure->body = NULL;

    env_free_frame(&new_frame);
    debug_end("/execute_lambda\n");
    return result;
}

void lambda_free(void* data) {
    debug_start("lambda_free - %llu\n", (size_t)data);
    if (data == NULL) {
        debug_end("/lambda_free\n");
        return;
    }

    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)data;
    debug("lambda_free/name - %llu\n", (size_t)lambda_closure->name);
    expr_free(&(lambda_closure->name));
    debug("lambda_free/body - %llu\n", (size_t)lambda_closure->body);
    expr_free(&(lambda_closure->body));
    debug("lambda_free/value - %llu\n", (size_t)lambda_closure->value);
    expr_free(&(lambda_closure->value));

    free_mem("lambda_free", data);
    debug_end("/lambda_free\n");
}

void* lambda_copy(void* data) {
    debug_start("lambda_copy\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)data;
    struct LambdaClosure* result =
        (struct LambdaClosure*)allocate_mem("lambda_copy", NULL,
            sizeof(struct LambdaClosure));
    assert(result != NULL);

    result->name = expr_copy(lambda_closure->name);
    result->body = expr_copy(lambda_closure->body);
    result->value = expr_copy(lambda_closure->value);
    result->static_env = lambda_closure->static_env;

    debug_end("/lambda_copy\n");
    return result;
}

Term_t make_let() {
    return term_make_abs(let_helper1, NULL, 0, lambda_free, lambda_copy);
}

Term_t let_helper1(EnvFrame_t env, Expr_t name, struct Closure closure) {
    debug_start("let_helper1\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("let_helper1", NULL,
        sizeof(struct LambdaClosure));
    lambda_closure->name = name;
    lambda_closure->body = NULL;
    lambda_closure->value = NULL;
    lambda_closure->static_env = NULL;
    Term_t result = term_make_abs(let_helper2, lambda_closure,
        sizeof(struct LambdaClosure), lambda_free, lambda_copy);
    debug_end("/let_helper1\n");
    return result;
}

Term_t let_helper2(EnvFrame_t env, Expr_t value, struct Closure closure) {
    debug_start("let_helper2\n");
    struct LambdaClosure* lambda_closure =
        (struct LambdaClosure*)allocate_mem("let_helper2", NULL,
        closure.size);
    lambda_closure->name = ((struct LambdaClosure*)closure.data)->name;
    ((struct LambdaClosure*)closure.data)->name = NULL;
    lambda_closure->body = NULL;
    lambda_closure->value = value;
    lambda_closure->static_env = env;
    Term_t result = term_make_abs(let_helper3, lambda_closure, closure.size,
        lambda_free, lambda_copy);
    debug_end("/let_helper2\n");
    return result;
}

Term_t let_helper3(EnvFrame_t env, Expr_t body, struct Closure closure) {
    debug_start("let_helper3\n");
    struct LambdaClosure* lambda_closure = (struct LambdaClosure*)closure.data;
    lambda_closure->body = body;
    Term_t result = execute_lambda(env, lambda_closure);
    debug_end("/let_helper3\n");
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
    return term_make_abs(binop_helper, NULL, 0, binop_free, binop_copy);
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
    debug_start("binop_helper1\n");
    struct MathBinopClosure* closure_data =
        (struct MathBinopClosure*)allocate_mem(NULL, NULL,
            sizeof(struct MathBinopClosure));
    size_t closure_size = sizeof(struct MathBinopClosure);
    closure_data->binop = binop;
    closure_data->operand1 = op1;
    Term_t result = term_make_abs(execute_binop, closure_data, closure_size,
        binop_free, binop_copy);
    debug_end("/binop_helper1\n");
    return result;
}

Term_t execute_binop(EnvFrame_t env, Expr_t op2, struct Closure closure) {
    debug_start("execute_binop\n");
    struct MathBinopClosure* math_binop_closure =
        (struct MathBinopClosure*)closure.data;
    enum BinOp binop = math_binop_closure->binop;

    Term_t t1 = eval(env, math_binop_closure->operand1);
    if (t1 == NULL) {
        debug_end("/execute_binop\n");
        return NULL;
    }
    math_binop_closure->operand1 = NULL;

    Term_t t2 = eval(env, op2);
    if (t2 == NULL) {
        debug_end("/execute_binop\n");
        return NULL;
    }
    op2 = NULL;

    // Check if the operands are numbers
    if (t1 == NULL || t2 == NULL) {
        debug("execute_binop: at least one operand is not evaluable\n");
        debug_end("/execute_binop\n");
        return NULL;
    }
    enum TermType t1type = term_get_type(t1);
    enum TermType t2type = term_get_type(t2);
    assert(t1type == ValTerm && t2type == ValTerm);

    struct Value t1val = term_get_value(t1);
    struct Value t2val = term_get_value(t2);
    assert(t1val.type == RationalVal && t2val.type == RationalVal);

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
    binop_result = NULL;
    term_free(&t1); term_free(&t2);
    debug_end("/execute_binop\n");
    return result;
}

void binop_free(void* data) {
    debug_start("binop_free\n");
    // Data can be null if the abstraction has never been used, as it is
    // initialized with a NULL data
    if (data == NULL) {
        debug_end("/binop_free\n");
        return;
    }

    struct MathBinopClosure* binop_closure = (struct MathBinopClosure*)data;
    if (binop_closure->operand1 != NULL) {
        expr_free(&(binop_closure->operand1));
    }
    free_mem("binop_free", data);
    debug_end("/binop_free\n");
}

void* binop_copy(void*data) {
    struct MathBinopClosure* binop_closure = (struct MathBinopClosure*)data;
    struct MathBinopClosure* result =
        (struct MathBinopClosure*)allocate_mem("binop_copy", NULL,
            sizeof(struct MathBinopClosure));
    assert(result != NULL);

    result->binop = binop_closure->binop;
    result->operand1 = expr_copy(binop_closure->operand1);

    return result;
}

// Term_t make_eq() {
//     // return term_make_abs();
// }

ErrorCode_t add_builtin(EnvFrame_t frame, char* name, Term_t term) {
    Expr_t expr = parse_from_str(name);
    env_add_entry(frame, expr, term);
    return Success;
}

// Create the ground environment, with the default lookup values and the
// built-in functions
EnvFrame_t env_make_default() {
    debug_start("env_make_default\n");
    EnvFrame_t env = env_make_empty_frame(NULL);
    if (env != NULL) {
        add_builtin(env, "lambda",  make_lambda());
        add_builtin(env, "let",     make_let());
        // add_builtin(env, "eq?",     make_eq());
        add_builtin(env, "+",       make_binop(ADD));
        add_builtin(env, "-",       make_binop(SUB));
        add_builtin(env, "*",       make_binop(MUL));
        add_builtin(env, "/",       make_binop(DIV));
    }
    debug_end("/env_make_default\n");
    return env;
}
