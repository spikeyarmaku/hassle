#ifndef _COMBINATORS_H_
#define _COMBINATORS_H_

#include <stdint.h>

#include "tree/tree.h"

struct Tree*    delta       ();

// Combinators
struct Tree*    cK          ();
struct Tree*    cI          ();
struct Tree*    cD          ();
struct Tree*    nD          (struct Tree*);
struct Tree*    cS          ();

// Boolean
struct Tree*    true        ();
struct Tree*    false       ();
struct Tree*    and         ();
struct Tree*    or          ();
struct Tree*    not         ();
// TODO cImplies, cNot, cIff - p. 32.

// Pair
struct Tree*    pair        ();
struct Tree*    nFirst      (struct Tree*);
struct Tree*    nSecond     (struct Tree*);
struct Tree*    fst         ();
struct Tree*    snd         ();

// Numbers
struct Tree*    nNat        (size_t);
struct Tree*    nPow        (struct Tree*, uint8_t);
struct Tree*    is_zero     ();
struct Tree*    nSucc       (struct Tree*);
struct Tree*    nPred       (struct Tree*);
struct Tree*    nQuery      (struct Tree*, struct Tree*, struct Tree*);
struct Tree*    is_leaf     ();
struct Tree*    is_stem     ();
struct Tree*    is_fork     ();

// Variable binding
struct Tree*    nBracket    (char*, struct Tree*);
BOOL            is_elem     (char*, struct Tree*);
struct Tree*    nStar       (char*, struct Tree*);
struct Tree*    nWait       (struct Tree*, struct Tree*);
struct Tree*    nWait1      (struct Tree*);
struct Tree*    self_apply  ();
struct Tree*    nZ          (struct Tree*);
struct Tree*    nSwap       (struct Tree*);
struct Tree*    nY2         (struct Tree*);

// Intensional
struct Tree*    nTag        (struct Tree*, struct Tree*);
struct Tree*    getTag      ();

struct Tree*    zero_rule           ();
struct Tree*    successor_rule      ();
struct Tree*    application_rule    ();
struct Tree*    empty_rule          ();
struct Tree*    substitution_rule   ();
struct Tree*    abstraction_rule    ();

struct Tree*    cV          ();
struct Tree*    cA          ();

// Exposed for debug purposes
struct Tree* _sym(char* symbol);
struct Tree* nY2t(struct Tree* tag, struct Tree* tree);
struct Tree* nTagWait(struct Tree* tree);

#endif