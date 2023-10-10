#ifndef _COMBINATORS_H_
#define _COMBINATORS_H_

#include <stdint.h>

#include "tree/term.h"

// Notation
struct Term*    delta       ();

// Combinators
struct Term*    cK          ();
struct Term*    cI          ();
struct Term*    cD          ();
struct Term*    nD          (struct Term*);
struct Term*    cS          ();

// Boolean
struct Term*    true        ();
struct Term*    false       ();
struct Term*    and         ();
struct Term*    or          ();
struct Term*    not         ();
// TODO cImplies, cNot, cIff - p. 32.

// Pair
struct Term*    pair        ();
struct Term*    nFirst      (struct Term*);
struct Term*    nSecond     (struct Term*);

// Numbers
struct Term*    nNat        (size_t);
struct Term*    nPow        (struct Term*, uint8_t);
struct Term*    is_zero     ();
struct Term*    nSucc       (struct Term*);
struct Term*    nPred       (struct Term*);
struct Term*    nQuery      (struct Term*, struct Term*, struct Term*);
struct Term*    is_leaf     ();
struct Term*    is_stem     ();
struct Term*    is_fork     ();

// Variable binding
struct Term*    nBracket    (char*, struct Term*);
BOOL            is_elem     (char*, struct Term*);
struct Term*    nStar       (char*, struct Term*);
struct Term*    nWait       (struct Term*, struct Term*);
struct Term*    nWait1      (struct Term*);
struct Term*    self_apply  ();
struct Term*    nZ          (struct Term*);
struct Term*    nSwap       (struct Term*);
struct Term*    nY2         (struct Term*);

// Intensional

#endif