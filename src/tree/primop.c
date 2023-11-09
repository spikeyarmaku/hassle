#include "primop.h"

// The reason primop_apply takes terms instead of primitive values is because
// later more operators might get added, which work on strings instead of
// numbers
struct Tree* primop_apply(uint8_t primop, struct Tree* tree0,
    struct Tree* tree1)
{
    if (tree_get_type(tree0) != TREE_TYPE_VALUE ||
        tree_get_type(tree1) != TREE_TYPE_VALUE)
    {
        printf("primop_apply: one of the operands is not a program\n");
        exit(1);
    }

    struct Program* prg0 = tree_get_value(tree0);
    struct Program* prg1 = tree_get_value(tree1);

    if (program_get_type(prg0) != VALUE_TYPE_RATIONAL ||
        program_get_type(prg1) != VALUE_TYPE_RATIONAL)
    {
        printf("primop_apply: one of the operands is not a number\n");
        exit(1);
    }

    Rational_t* num0 = value_get_rat(program_get_value(prg0));
    Rational_t* num1 = value_get_rat(program_get_value(prg1));
    switch (primop) {
        case Add: {
            return
                tree_make_value(program_make(
                    value_make_rat(rational_add(num0, num1)), NULL, NULL));
        }
        case Sub: {
            return
                tree_make_value(program_make(
                    value_make_rat(rational_sub(num0, num1)), NULL, NULL));
        }
        case Mul: {
            return
                tree_make_value(program_make(
                    value_make_rat(rational_mul(num0, num1)), NULL, NULL));
        }
        case Div: {
            return
                tree_make_value(program_make(
                    value_make_rat(rational_div(num0, num1)), NULL, NULL));
        }
        case Eq: {
            return rational_is_equal(num0, num1) == TRUE ? true() : false();
        }
        default: {
            return NULL;
        }
    }
}
