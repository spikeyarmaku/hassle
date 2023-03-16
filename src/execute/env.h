#ifndef _ENV_H_
#define _ENV_H_

#include <stdint.h>
#include <ctype.h>

#include "global.h"
#include "memory.h"

#include "execute\term.h"
#include "rational\rational.h"

struct Entry {
    Expr expr;          // The name of the stored value
    struct Term term;   // The stored value itself
};

struct EnvFrame {
    size_t entry_count;
    struct Entry* mapping;
    struct EnvFrame* parent;
};

struct Env {
    struct EnvFrame* current_frame;
    struct Dict* dict;
};

// Parse number and string
struct Term     default_rules       (Expr, struct Dict*);

// Return the term assigned to this expression in the given environment
struct Term     env_lookup          (struct Env, Expr);

// Take a list, and return the term corresponding to the longest sublist
struct Term*    find_longest_match  (struct Env, Expr, size_t*);

// Extend
ErrorCode       add_entry           (struct Env, Expr, struct Term);
ErrorCode       add_frame           (struct Env*, Expr, struct Term);
void            remove_last_frame   (struct Env*);

#endif