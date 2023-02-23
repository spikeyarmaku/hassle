#ifndef _ENV_H_
#define _ENV_H_

struct Expr; // Forward declaration

struct Entry {
    struct Expr* expr;
    struct Term* term;
    struct Entry* next;
};

struct Env {
    struct Entry* mapping;
    struct Term* (*rule)(struct Expr* expr); // for things like parsing a number
    struct Env* parent;
};

#endif