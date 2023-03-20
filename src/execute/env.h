#ifndef _ENV_H_
#define _ENV_H_

#include <stdint.h>
#include <ctype.h>

#include "global.h"
#include "memory.h"

#include "execute\term.h"
#include "rational\rational.h"

/*
Env usage:

Construction:
  To construct an env,

Destruction:
  
*/

struct Entry {
    Expr expr;          // The name of the stored value
    struct Term term;   // The stored value itself
};

struct _EnvFrame {
    size_t entry_count;
    struct Entry* mapping;
    struct _EnvFrame* parent;
};

typedef struct _EnvFrame* EnvFrame;

struct _Env {
    EnvFrame current_frame;
    struct Dict dict;
};

typedef struct _Env* Env;

Env             make_empty_env      ();
void            free_env            (Env);

// Parse number and string
struct Term     default_rules       (Expr, struct Dict);

// Return the term assigned to this expression in the given environment
struct Term     env_lookup          (Env, Expr);

// Take a list, and return the term corresponding to the longest sublist
struct Term*    find_longest_match  (Env, Expr, size_t*);

// Extend
ErrorCode       add_entry           (Env, Expr, struct Term);
ErrorCode       add_empty_frame     (Env);
void            remove_last_frame   (Env);

#endif