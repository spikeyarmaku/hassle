#ifndef _ENV_H_
#define _ENV_H_

#include <stdint.h>
#include <ctype.h>

#include "global.h"
#include "memory.h"

#include "execute\term_dict.h"
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

For memory efficiency reasons, an EnvDict_t type is introduced, which contains all
symbols and all terms that have been defined so far in the current frame. The
mappings in the frame will only contain an ID to those symbols and terms.

The reason there is no single global dict for the whole environment is that if
a frame is deleted, it is more memory-efficient to delete their bindings and
then recreate some of them later should the need arise, than trying to figure
out which mappings are no longer needed.
*/

struct Entry {
    Expr_t expr;                          // The name's ID
    uint8_t term_id[SYMBOL_ID_BYTES];   // The value's ID
};

struct _EnvDict {
    SymbolDict_t symbol_dict;
    TermDict_t term_dict;
};
typedef struct _EnvDict* EnvDict_t;

struct _EnvFrame {
    size_t entry_count;
    struct Entry* mapping;
    struct _EnvFrame* parent;
    EnvDict_t env_dict;
};
typedef struct _EnvFrame* EnvFrame_t;

EnvDict_t       make_empty_env_dict      ();
void            free_env_dict           (EnvDict_t*);

EnvFrame_t      make_empty_frame        (EnvFrame_t);
void            free_frame              (EnvFrame_t*);

// Parse number and string
struct Term     default_rules           (Expr_t, SymbolDict_t);

// Return the term assigned to this expression in the given environment
struct Term     env_lookup_term_by_expr (EnvFrame_t, Expr_t);
// struct Term     env_lookup_term_by_id   (EnvFrame_t, Expr_t);
char*           env_lookup_symbol_by_id (Expr_t, EnvFrame_t);

// Take a list, and return the term corresponding to the longest sublist
struct Term*    find_longest_match      (EnvFrame_t, Expr_t, size_t*);

void            print_env_frame         (EnvFrame_t);

// Extend
enum ErrorCode     add_entry               (EnvFrame_t, Expr_t, struct Term);
void            free_entry              (struct Entry);

int             print_expr              (Expr_t, EnvFrame_t, char*);
void            print_term              (char*, struct Term, EnvFrame_t);

#endif