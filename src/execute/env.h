#ifndef _ENV_H_
#define _ENV_H_

#include <stdint.h>
#include <ctype.h>

#include "global.h"
#include "memory.h"

#include "rational\rational.h"

/*
Env usage:

Construction:
  To construct an env, // TODO

Destruction:
  // TODO
*/

/*
SICP page 320 defines the environment model as a tree of mappings, where
mappings are grouped into frames, and these frames point to their parent.
*/

struct Entry {
    Expr_t name;
    struct Term value;
};

struct EnvFrame {
    size_t entry_count;
    struct Entry* mapping;
    struct _EnvFrame* parent;
    EnvDict_t env_dict;
};
typedef struct EnvFrame* EnvFrame_t;

EnvFrame_t      env_make_empty_frame    (EnvFrame_t);
void            env_free_frame          (EnvFrame_t*);

// Parse number and string
struct Term     env_default_rules       (Expr_t);

// Return the term assigned to this expression in the given environment
struct Term     env_lookup_term         (EnvFrame_t, Expr_t);

// Take a list, and return the term corresponding to the longest sublist
struct Term*    env_find_longest_match  (EnvFrame_t, Expr_t, size_t*);

void            env_print_frame         (EnvFrame_t);

// Extend
ErrorCode_t     env_add_entry           (EnvFrame_t, Expr_t, struct Term);
void            env_free_entry          (struct Entry);

#endif