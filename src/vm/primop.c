#include "primop.h"

#include "term.h" // keeping this include here avoids a circular dependency
#include "frame.h"

Closure_t* _primop_vau          (Closure_t*, Closure_t*);
Closure_t* _primop_rational_op  (Closure_t*, Closure_t*,
    Rational_t*(*)(Rational_t*, Rational_t*));
Closure_t* _primop_eq           (Closure_t*, Closure_t*);

uint8_t primop_get_arity(enum PrimOp primop) {
    return 2;
}

Closure_t* primop_apply(enum PrimOp primop, Closure_t** args) {
    switch (primop) {
        case Vau: {
            return _primop_vau(args[0], args[1]);
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

Closure_t* _primop_vau(Closure_t* closure1, Closure_t* closure2) {
    Term_t* term1 = closure_get_term(closure1);
    
    assert(term_get_type(term1) == PrimValTerm);
    
    PrimVal_t* primval1 = term_get_primval(term1);

    assert(primval_get_type(primval1) == SymbolValue);

    return
        closure_make(
            term_make_abs(
                primval_get_symbol(primval1),
                term_make_app(
                    term_make_decode_raw(),
                    closure_get_term(closure2))),
            closure_get_frame(closure2));
}

Closure_t* _primop_rational_op(Closure_t* closure1, Closure_t* closure2,
    Rational_t*(*rational_op)(Rational_t*, Rational_t*))
{
    Term_t* term1 = closure_get_term(closure1);
    Term_t* term2 = closure_get_term(closure2);
    Frame_t* frame = closure_get_frame(closure2);
    
    assert(term_get_type(term1) == PrimValTerm &&
        term_get_type(term2) == PrimValTerm);
    
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
    // closure_free(closure1);
    // closure_free(closure2);

    return result;
}

Closure_t* _primop_eq(Closure_t* closure1, Closure_t* closure2) {
    Term_t* term1 = closure_get_term(closure1);
    Term_t* term2 = closure_get_term(closure2);
    Frame_t* frame = closure_get_frame(closure1);
    
    assert(term_get_type(term1) == PrimValTerm &&
        term_get_type(term1) == PrimValTerm);

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
            case ReferenceValue: {
                // TODO eval both closures and compare the terms?
                return NULL;
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
