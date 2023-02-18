#ifndef _EXPR_H_
#define _EXPR_H_

#include <stdlib.h>
#include <string.h>

// Expr: Symbol or List

enum ExprType {ExprAtom, ExprList};

struct Atom {
    long int char_count;
    char *symbol;
};

// https://benpaulhanna.com/writing-a-game-in-c-parsing-s-expressions.html
struct Expr {
  struct Expr *next;
  struct Expr* prev;
  enum ExprType type;
  union {
    struct Expr *list;
    struct Atom atom;
  };
};

struct Expr* append(struct Expr*);
struct Expr* get_head(struct Expr*);
void destroy_expr(struct Expr*);

#endif