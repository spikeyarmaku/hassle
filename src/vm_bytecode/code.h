// NOTE: if we make K and eraser different agents, we could probably always
// make equations where the agents in the active pairs of the rhs is always in
// the correct order, so there is no need for comparing them in the evaluation
// logic.
// Basically, we could divide the agents into two groups, such that agents from
// group 1 would always appear on the rhs of an active pair, and agents from
// group 2 would wlays appear on the lhs

/*
A way to convert (x, y) pairs into indexes in a flattened triangle matrix:

let us have an N x N matrix such as (n = 5):

          X
      0  1  2  3  4
    +--------------
  0 | 0  1  2  3  4
  1 | X  5  6  7  8
Y 2 | X  X  9 10 11
  3 | X  X  X 12 13
  4 | X  X  X  X 14

First, we make sure that x >= y. If it's not true, we swap x and y.
Then we get the correct index by this formula: y * n + x - (y * (y + 1)) / 2)

the size of the flattened array is n * (n + 1) / 2


           X
      K  S  F  @  #
    +----------------
  K | KK KS KF K@ K#
  S | X  SS SF S@ S#
Y F | X  X  FF F@ F#
  @ | X  X  X  @@ @#
  # | X  X  X  X  ##

Rules SS, @@, @# and ## are not implemented

The rules in inpla's notation (https://github.com/inpla/inpla):
K >< E => ;
K >< D(x, y) => K~x, K~y;
K >< A(x, r) => S(x)~r;
K >< T(y, z, r) => y~r, z~E;
S(x) >< E => x~E;
S(x) >< D(y, z) => x~D(a, b), S(a)~y, S(b)~z;
S(x) >< A(y, r) => F(x, y)~r;
S(x) >< T(y, z, r) => z~D(a, b), x~A(a, c), y~A(b, A(c, r));
F(x, y) >< E => x~E, y~E;
F(w, x) >< D(y, z) => F(a, b)~z, F(c, d)~y, w~D(c, a), x~D(d, b);
F(x, y) >< A(z, r) => x~T(y, z, r);
F(w, x) >< T(y, z, r) => z~A(w, A(x, r)), y~E;
*/

#ifndef _CODE_H_
#define _CODE_H_

#include <stdint.h>

#include "global.h"

#define VAR(x) (VARIABLE_INDEX_START + x)
#define L(x) (x)
#define R(x) (MAX_AUX_PORT_NUM + x)

extern const uint8_t Arities[MAX_AGENT_ID];

extern const uint8_t rule_k_e[];
extern const uint8_t rule_k_d[];
extern const uint8_t rule_k_a[];
extern const uint8_t rule_k_t[];
extern const uint8_t rule_s_e[];
extern const uint8_t rule_s_d[];
extern const uint8_t rule_s_a[];
extern const uint8_t rule_s_t[];
extern const uint8_t rule_f_e[];
extern const uint8_t rule_f_d[];
extern const uint8_t rule_f_a[];
extern const uint8_t rule_f_t[];

extern const uint8_t* CodeTable[];
extern const char* CodeNameTable[];

#endif