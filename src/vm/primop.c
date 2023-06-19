#include "primop.h"

#include "term.h" // keeping this include here avoids a circular dependency
#include "frame.h"

Closure_t* _primop_lambda      (Closure_t*, Closure_t*);
Closure_t* _primop_eval        (Closure_t*);
Closure_t* _primop_rational_op (Closure_t*, Closure_t*,
    Rational_t*(*)(Rational_t*, Rational_t*));
Closure_t* _primop_eq          (Closure_t*, Closure_t*);

uint8_t primop_get_arity(enum PrimOp primop) {
    if (primop == Eval) {
        return 1;
    }
    return 2;
}

Closure_t* primop_apply(enum PrimOp primop, Closure_t** args) {
    switch (primop) {
        case Lambda: {
            return _primop_lambda(args[0], args[1]);
        }
        case Eval: {
            return _primop_eval(args[0]);
        }
        case Add: {
            return _primop_rational_op(args[0], args[1], &rational_add);
        }
        case Sub: {
            return _primop_rational_op(args[0], args[1], &rational_sub);
        }
        case Mul: {
            return _primop_rational_op(args[0], args[1], &rational_mul);
        }
        case Div: {
            return _primop_rational_op(args[0], args[1], &rational_div);
        }
        case Eq: {
            return _primop_eq(args[0], args[1]);
        }
        default: assert(FALSE); return NULL;
    }
}

// Return an abstraction with body marked to be evaluated
Closure_t* _primop_lambda(Closure_t* closure1, Closure_t* closure2) {
    Term_t* term1 = closure_get_term(closure1);
    
    assert(term_get_type(term1) == PrimvalTerm);
    
    PrimVal_t* primval1 = term_get_primval(term1);

    assert(primval_get_type(primval1) == SymbolValue);

    Closure_t* evaled2 = _primop_eval(closure2);

    return
        closure_make(
            term_make_abs(
                primval_get_symbol(primval1), closure_get_term(evaled2)),
            closure_get_frame(closure2));
}

// If the argument is a value, return the value. If the argument is a symbol,
// return its value. If the argument is an application, evaluate the head, and
// apply the argument as-is.
Closure_t* _primop_eval(Closure_t* closure) {
    Term_t* term = closure_get_term(closure);
    Frame_t* frame = closure_get_frame(closure);
    
    if (term_get_type(term) == PrimvalTerm) {
        PrimVal_t* primval = term_get_primval(term);
        if (primval_get_type(primval) == SymbolValue) {
            // if symbol, look up value of symbol
            return frame_lookup(frame, primval_get_symbol(primval));
        } else {
            // if value, return value
            return closure;
        }
    } else {
        // if app, eval head and apply it to the rest
        assert(term_get_type(term) == LazyAppTerm);
        Term_t* term1 = term_get_app_term1(term);
        Term_t* term2 = term_get_app_term2(term);
        Closure_t* evaled_head = _primop_eval(closure_make(term1, frame));
        return
            closure_make(
                term_make_lazy_app(closure_get_term(evaled_head), term2),
                frame);
    }
}

Closure_t* _primop_rational_op(Closure_t* closure1, Closure_t* closure2,
    Rational_t*(*rational_op)(Rational_t*, Rational_t*))
{
    Term_t* term1 = closure_get_term(closure1);
    Term_t* term2 = closure_get_term(closure2);
    Frame_t* frame = closure_get_frame(closure2);
    
    assert(term_get_type(term1) == PrimvalTerm &&
        term_get_type(term2) == PrimvalTerm);
    
    // Extract the numbers
    PrimVal_t* val1 = term_get_primval(term1);
    PrimVal_t* val2 = term_get_primval(term2);

    assert(primval_get_type(val1) == RationalValue &&
        primval_get_type(val2) == RationalValue);

    Rational_t* rat1 = primval_get_rational(val1);
    Rational_t* rat2 = primval_get_rational(val2);

    // Perform the operation
    Rational_t* rat_result = rational_op(rat1, rat2);
    Closure_t* result = closure_make(
        term_make_primval(primval_make_rational(rat_result)), frame);
    
    // Cleanup
    closure_free(closure1);
    closure_free(closure2);

    return result;
}

Closure_t* _primop_eq(Closure_t* closure1, Closure_t* closure2) {
    Term_t* term1 = closure_get_term(closure1);
    Term_t* term2 = closure_get_term(closure2);
    Frame_t* frame = closure_get_frame(closure1);
    
    assert(term_get_type(term1) == PrimvalTerm &&
        term_get_type(term1) == PrimvalTerm);

    // Extract the values
    PrimVal_t* val1 = term_get_primval(term1);
    PrimVal_t* val2 = term_get_primval(term2);

    if (primval_get_type(val1) != primval_get_type(val2)) {
        return closure_make(term_make_false(), frame);
    } else {
        switch (primval_get_type(val1)) {
            case RationalValue: {
                Rational_t* rat1 = primval_get_rational(val1);
                Rational_t* rat2 = primval_get_rational(val2);
                if (rational_is_equal(rat1, rat2)) {
                    return closure_make(term_make_true(), frame);
                } else {
                    return closure_make(term_make_false(), frame);
                }
            }
            case StringValue: {
                char* str1 = primval_get_string(val1);
                char* str2 = primval_get_string(val2);
                if (strcmp(str1, str2) == 0) {
                    return closure_make(term_make_true(), frame);
                } else {
                    return closure_make(term_make_false(), frame);
                }
            }
            case SymbolValue: {
                char* sym1 = primval_get_symbol(val1);
                char* sym2 = primval_get_symbol(val2);
                if (strcmp(sym1, sym2) == 0) {
                    return closure_make(term_make_true(), frame);
                } else {
                    return closure_make(term_make_false(), frame);
                }
            }
            default: return closure_make(term_make_false(), frame);
        }
    }
}

