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
BOOL            occurs_t    (char*, struct Tree*);
BOOL            occurs_p    (char*, struct Program*);
struct Tree*    nStar       (char*, struct Tree*);
struct Tree*    nWait       (struct Tree*, struct Tree*);
struct Tree*    nWait1      (struct Tree*);
struct Tree*    self_apply  ();
struct Tree*    nZ          (struct Tree*);
struct Tree*    nSwap       (struct Tree*);
struct Tree*    nY2         (struct Tree*);
struct Tree*    plus        ();

// Intensional
struct Tree*    nTag        (struct Tree*, struct Tree*);
struct Tree*    get_tag     ();

struct Tree*    zero_rule           ();
struct Tree*    successor_rule      ();
struct Tree*    application_rule    ();
struct Tree*    empty_rule          ();
struct Tree*    substitution_rule   ();
struct Tree*    abstraction_rule    ();

struct Tree*    cV          ();
struct Tree*    cA          ();

// Own experiments
struct Tree*    inc         ();

// Exposed for debug purposes
struct Tree*    _ref        (char*);
// struct Tree*    _str        (char*);
struct Tree*    nY2t        (struct Tree*, struct Tree*);
struct Tree*    nTagWait    (struct Tree*);

#endif