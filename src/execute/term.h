#ifndef _TERM_H_
#define _TERM_H_

#include "parse\expr.h"
#include "rational\rational.h"

#include "memory.h"

struct Term;
struct _Env;
struct _EnvFrame; // Forward declaration

enum TermType {ValTerm, ExprTerm, AbsTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string;
        Rational_t rational;
    };
};

struct Abstraction {
    enum ErrorCode (*apply)(struct _EnvFrame*, Expr_t, void*, struct Term*);
    void* closure;
};

struct Term {
    enum TermType type;
    union {
        struct Value value;
        Expr_t expr;
        // enum ErrorCode (*apply)(struct EnvFrame*, Expr_t, struct Term*);
        struct Abstraction abs;
    };
};

BOOL        is_equal_term           (struct Term, struct Term);

struct Term make_number             (Rational_t);
struct Term make_string             (char*);
struct Term make_expr               (Expr_t);

void        free_term               (struct Term);

#endif