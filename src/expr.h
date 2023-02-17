#ifndef _EXPR_H_
#define _EXPR_H_

#include <string.h>

// Expr: Symbol or List

enum ExprType { ExprSymbol, ExprList };

struct Expr {
    enum ExprType expr_type;
    union {
        struct Symbol s;
        struct List l;
    };
};

struct Symbol {
    int char_count;
    char* symbol;
};

struct List {
    int child_count;
    struct Expr* children;
};

struct Expr create_list();
struct Expr create_symbol(char* symbol_name);

#endif