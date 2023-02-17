#include "expr.h"

struct Expr create_list() {
    struct Expr expr;
    expr.expr_type = ExprList;

    struct List list;
    list.child_count = 0;
    expr.l = list;

    return expr;
}

struct Expr create_symbol(char* symbol_name) {
    struct Expr expr;
    expr.expr_type = ExprSymbol;

    struct Symbol symbol;
    symbol.symbol = symbol_name;
    symbol.char_count = strlen(symbol_name);
    expr.s = symbol;
    return expr;
}