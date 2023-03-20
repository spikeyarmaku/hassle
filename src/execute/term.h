#ifndef _TERM_H_
#define _TERM_H_

#include "parse\expr.h"
#include "rational\rational.h"

struct Term;
struct _Env;
struct EnvFrame; // Forward declaration

enum TermType {ValTerm, ExprTerm, AbsTerm};
enum ValType {StringVal, RationalVal};

struct Value {
    enum ValType type;
    union {
        char* string;
        Rational rational;
    };
};

struct Abstraction {
    ErrorCode (*apply)(struct _Env*, Expr, void*, struct Term*);
    void* closure;
};

struct Term {
    enum TermType type;
    union {
        struct Value value;
        Expr expr;
        // ErrorCode (*apply)(struct EnvFrame*, Expr, struct Term*);
        struct Abstraction abs;
    };
};

struct Term make_number (Rational);
struct Term make_string (char*);
struct Term make_expr   (Expr);

#endif