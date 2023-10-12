#include "primop.h"

// The reason primop_apply takes terms instead of primitive values is because
// later more operators might get added, which work on strings instead of
// numbers
struct Term* primop_apply(uint8_t primop, struct Term* term1,
    struct Term* term2)
{
    if (term_type(term1) != TERM_TYPE_RATIONAL ||
        term_type(term2) != TERM_TYPE_RATIONAL)
    {
        printf("primop_apply: one of the operands is not a number\n");
        exit(1);
    }

    Rational_t* num1 = term_get_rat(term1);
    Rational_t* num2 = term_get_rat(term2);
    switch (primop) {
        case Add: {
            return term_make_rat(rational_add(num1, num2));
        }
        case Sub: {
            return term_make_rat(rational_sub(num1, num2));
        }
        case Mul: {
            return term_make_rat(rational_mul(num1, num2));
        }
        case Div: {
            return term_make_rat(rational_div(num1, num2));
        }
        case Eq: {
            return rational_is_equal(num1, num2) == TRUE ? true() : false();
        }
        default: {
            return NULL;
        }
    }
}
