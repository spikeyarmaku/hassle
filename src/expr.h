#ifndef _EXPR_H_
#define _EXPR_H_

#include <string.h>

#include "memory.h"

// Expr: Symbol or List

enum ExprType {ExprAtom, ExprList};

// https://benpaulhanna.com/writing-a-game-in-c-parsing-s-expressions.html
struct Expr {
  struct Expr *next;
  struct Expr *prev;
  enum ExprType type;
  union {
    struct Expr *list;
    char *symbol;
  };
};

struct Expr* append(struct Expr*);
struct Expr* get_head(struct Expr*);
void destroy_expr(struct Expr*);

#endif