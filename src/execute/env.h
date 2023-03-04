#ifndef _ENV_H_
#define _ENV_H_

#include <stdint.h>
#include <ctype.h>

#include "execute\term.h"
#include "rational\rational.h"

struct Entry {
    struct Expr expr; // The name of the stored value
    struct Term term; // The stored value itself
    struct Entry* next;
};

// TODO There are design problems around lookup - it doesn't match toxicscript's
// design
struct Env {
    struct Entry* mapping;
    struct Env* parent;
};

struct Term     default_rules       (struct Expr); // for things like parsing a number

struct Term     env_lookup          (struct Env*, struct Expr);
// Take a list, and return the term corresponding to the longest sublist
struct Term*    find_longest_match  (struct Env*, struct Expr, size_t*);

// struct Env make_default_env();

#endif