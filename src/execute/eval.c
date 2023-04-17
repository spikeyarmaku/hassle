/*
a stack is a part of a frame, containing values

- put the expression to be evaluated on the stack as an expr value
- call eval

eval pops the top of the stack, and if it is a value, it calls apply. But if it
is an expression instead, it evaluates it. If it is a symbol, it looks up its
value, and if it is a combination, it puts the constituents onto the stack in
reverse order (so the first subexpr is on top).

when there is nothing to apply the topmost element to (because there is no other
element), the calculation has finished.
*/

#include "eval.h"

void            eval_expr           (EnvFrame_t, Expr_t);
enum EvalState  apply               (EnvFrame_t, Term_t);

// Keep calling eval_step until the evaluation finishes
Term_t eval(EnvFrame_t frame, Expr_t expr) {
    debug_start("eval - expr ptr: %llu\n", (size_t)expr);
    assert(expr != NULL);

    // env_print_frame(frame);

    Term_t expr_term = term_make_expr(expr);
    stack_push(env_get_stack(frame), expr_term);
    expr = NULL; // from this point, the stack owns `expr`

    enum EvalState eval_state;
    do {
        eval_state = eval_step(frame);
    } while (eval_state != EvalFinished);

    Term_t result = stack_pop(env_get_stack(frame));
    debug_end("/eval\n");
    return result;
}

// Perform one step of evaluation, calling apply if necessary
enum EvalState eval_step(EnvFrame_t frame) {
    debug_start("eval_step\n");

    // stack_print(env_get_stack(frame));

    // Pop the top of the stack
    Term_t term = stack_pop(env_get_stack(frame));
    assert(term != NULL);

    // If it is a value or abstraction, call apply. Otherwise, check the expr.
    enum TermType type = term_get_type(term);
    enum EvalState result;
    if (type == ExprTerm) {
        debug("type == ExprTerm\n");
        // If it is a simple expression, look up its value. Otherwise,
        // break it into its constituents and place them onto the stack.
        Expr_t expr = term_get_expr(term);
        assert(expr != NULL);
        eval_expr(frame, expr_copy(expr));
        result = EvalRunning;
    } else {
        debug("type != ExprTerm\n");
        // Otherwise just apply it to the next element in the stack
        result = apply(frame, term_copy(term));
    }
    term_free(&term);
    debug_end("/eval_step\n");
    return result;
}

// Look up the value of an expression, or break down a compound expression, and
// put its constituents on the stack in reverse order
// TODO Can it cache expressions?
void eval_expr(EnvFrame_t frame, Expr_t expr) {
    debug_start("eval_expr\n");
    // expr_print(expr); debug("\n");
    assert(expr != NULL);
    if (expr_is_list(expr)) {
        debug("Expression is list\n");
        // Put the constituents onto the stack in reverse order
        size_t child_count = expr_get_child_count(expr);
        for (size_t i = child_count; i > 0; i--) {
            assert(expr_get_child(expr, i - 1) != NULL);
            Term_t temp =
                term_make_expr(expr_copy(expr_get_child(expr, i - 1)));
            stack_push(env_get_stack(frame), temp);
        }
    } else {
        debug("Expression is atom\n");
        // If it is an atom, look up its value and place it on the stack
        Term_t term = env_lookup_term(frame, expr);
        stack_push(env_get_stack(frame), term);
    }
    expr_free(&expr);
    debug_end("/eval_expr\n");
}

// Pop a term from the stack and apply it to the term. If the stack is empty,
// signal the end of the evaluation instead.
enum EvalState apply(EnvFrame_t frame, Term_t term) {
    debug_start("apply - term: %llu\n", (size_t)term);
    assert(term != NULL);

    // If the stack is empty, push the term on the stack and signal the end of
    // the evaluation
    if (stack_is_empty(env_get_stack(frame))) {
        stack_push(env_get_stack(frame), term);
        debug_end("/apply\n");
        return EvalFinished;
    }

    // Otherwise, check if the term is an abstraction
    enum TermType type = term_get_type(term);
    // If it is not an abstraction, signal an error
    assert(type == AbsTerm);

    // Pop the top element off the stack
    Term_t arg = stack_pop(env_get_stack(frame));
    assert(arg != NULL);

    // Check its type
    type = term_get_type(arg);
    // If it is not an expression, signal an error
    assert(type == ExprTerm);
    Expr_t arg_expr = term_get_expr(arg);
    assert(arg_expr != NULL);

    // get the abstraction of the term
    struct Abstraction abs = term_get_abs(term);

    // Apply the abstraction to the popped-off term's expression
    Term_t result = abs.apply(frame, expr_copy(arg_expr), abs.closure);
    assert(result != NULL);

    // Push it on the stack
    stack_push(env_get_stack(frame), result);

    debug("apply - free arg\n");
    term_free(&arg);
    debug("apply - free term\n");
    term_free(&term);
    debug_end("/apply\n");
    return EvalRunning;
}
