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
    //expr_print(expr); debug("\n");

    Term_t expr_term = term_make_expr(expr);
    stack_push(env_get_stack(frame), expr_term);
    enum EvalState eval_state = eval_step(frame);
    while (eval_state != EvalFinished) {
        eval_state = eval_step(frame);
    }
    
    Term_t result = stack_pop(env_get_stack(frame));
    debug_end("/eval\n");
    return result;
}

// Perform one step of evaluation, calling apply if necessary
enum EvalState eval_step(EnvFrame_t frame) {
    debug_start("eval_step\n");
    // Pop the top of the stack
    Term_t term = stack_pop(env_get_stack(frame));
    if (term == NULL) {
        debug_end("/eval_step\n");
        return EvalFinished;
    }

    // If it is a value or abstraction, call apply. Otherwise, check the expr.
    enum TermType type = term_get_type(term);
    enum EvalState result;
    if (type == ExprTerm) {
        // If it is a simple expression, look up its value. Otherwise,
        // break it into its constituents and place them onto the stack.
        Expr_t expr = term_get_expr(term);
        eval_expr(frame, expr);
        result = EvalRunning;
    } else {
        // Otherwise just apply it to the next element in the stack
        result = apply(frame, term);
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
    if (expr_is_list(expr)) {
        debug("Expression is list\n");
        // Put the constituents onto the stack in reverse order
        size_t child_count = expr_get_child_count(expr);
        for (size_t i = child_count; i > 0; i--) {
            stack_push(env_get_stack(frame),
                term_make_expr(expr_get_child(expr, i - 1)));
        }
    } else {
        debug("Expression is atom\n");
        // If it is an atom, look up its value and place it on the stack
        Term_t term = env_lookup_term(frame, expr);
        stack_push(env_get_stack(frame), term);
    }
    debug_end("/eval_expr\n");
}

// Pop a term from the stack and apply it to the term. If the stack is empty,
// signal the end of the evaluation instead.
enum EvalState apply(EnvFrame_t frame, Term_t term) {
    debug_start("apply\n");

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

    // get the abstraction of the term
    struct Abstraction abs = term_get_abs(term);

    // Apply the abstraction to the popped of term's expression
    Term_t result = abs.apply(frame, arg_expr, abs.closure);

    // Push it on the stack
    stack_push(env_get_stack(frame), result);
    // term_free(&term);
    term_free(&arg);
    debug_end("/apply\n");
    return EvalRunning;
}
