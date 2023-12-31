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

First, we make sure that x <= y. If it's not true, we swap x and y.
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
K >< K => ;
K >< S(x) => x~K;
K >< F(x,y) => x~K, y~K;
K >< App(x, r) => r~S(x);
K >< Delta(y, z, r) => y~r, z~K;
S(x) >< F(y, z) => x~F(a, b), y~S(a), z~S(b);
S(x) >< App(y, r) => r~F(x, y);
S(x) >< Delta(y, z, r) => z~F(a, b), x~App(a, c), y~App(b, App(c, r));
F(w, x) >< F(y, z) => w~F(a, c), x~F(d, b), y~F(d, a), z~F(b, c);
F(x, y) >< App(z, r) => x~Delta(y, z, r);
F(w, x) >< Delta(y, z, r) => z~App(w, App(x, r)), y~K;
*/

#ifndef _CODE_H_
#define _CODE_H_

#include <stdint.h>

#include "global.h"

#define VAR(x) (VARIABLE_INDEX_START + x)
#define L(x) (x)
#define R(x) (MAX_AUX_PORT_NUM + x)

extern const uint8_t Arities[MAX_AGENT_ID];

extern const uint8_t rule_k_k[];
extern const uint8_t rule_k_s[];
extern const uint8_t rule_k_f[];
extern const uint8_t rule_k_app[];
extern const uint8_t rule_k_delta[];
extern const uint8_t rule_s_f[];
extern const uint8_t rule_s_app[];
extern const uint8_t rule_s_delta[];
extern const uint8_t rule_f_f[];
extern const uint8_t rule_f_app[];
extern const uint8_t rule_f_delta[];

extern const uint8_t* CodeTable[];

#endif