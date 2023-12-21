#ifndef _CODE_H_
#define _CODE_H_

#include <stdint.h>

#include "global.h"

// char Symbols[MAX_AGENT_ID] = {0, 'K', 'S', 'F', '@', '#'};
const uint8_t Arities[MAX_AGENT_ID] = {1, 0, 1, 2, 2, 3};

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

#define VAR(x) (VARIABLE_INDEX_START + x)
#define L(x) (x)
#define R(x) (MAX_AUX_PORT_NUM + x)

// K >< K => ;
const uint8_t rule_k_k[] = {OP_RETURN};

// K >< S(x) => x~K;
const uint8_t rule_k_s[] = {
    // vm_push_eq(vm, agent0, agent1->ports[0]);
    OP_MKAGENT, VAR(0), ID_K,
    OP_PUSH, VAR(0), R(0),
    OP_RETURN};

// K >< F(x,y) => x~K, y~K;
const uint8_t rule_k_f[] = {
    // struct Agent* agent_K_0 = agent_make(ID_K);
    OP_MKAGENT, VAR(0), ID_K,
    // struct Agent* agent_K_1 = agent_make(ID_K);
    OP_MKAGENT, VAR(1), ID_K,
    // vm_push_eq(vm, agent_K_0, agent1->ports[0]);
    OP_PUSH, VAR(0), R(0),
    // vm_push_eq(vm, agent_K_1, agent1->ports[1]);
    OP_PUSH, VAR(1), R(1),
    OP_RETURN};

// K >< App(x, r) => r~S(x);
const uint8_t rule_k_app[] = {
    // struct Agent* agent_s = agent_make(ID_S);
    OP_MKAGENT, VAR(0), ID_S,
    // agent_s->ports[0] = agent1->ports[0];
    OP_MOVEP, VAR(0), 0, R(0),
    // vm_push_eq(vm, agent1->ports[1], agent_s);
    OP_PUSH, R(1), VAR(0),
    OP_RETURN};

// K >< Delta(y, z, r) => y~r, z~K;
const uint8_t rule_k_delta[] = {
    // vm_push_eq(vm, agent1->ports[0], agent1->ports[2]);
    OP_MKAGENT, VAR(0), ID_K,
    OP_PUSH, R(0), R(2),
    OP_PUSH, VAR(0), R(1), // TODO maybe add a FREE_REC to recursively free the agent R(1)
    OP_RETURN};

// S(x) >< F(y, z) => x~F(a, b), y~S(a), z~S(b);
const uint8_t rule_s_f[] = {
    // struct Agent* agent_S_0 = agent_make(ID_S);
    OP_MKAGENT, VAR(0), ID_S,
    // struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // agent_S_0->ports[0] = name_a;
    OP_MOVEP, VAR(0), 0, VAR(1),
    // vm_push_eq(vm, agent1->ports[0], agent_S_0);
    OP_PUSH, R(0), VAR(0),

    // struct Agent* agent_S_1 = agent_make(ID_S);
    OP_MKAGENT, VAR(2), ID_S,
    // struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(3),
    // agent_S_1->ports[0] = name_b;
    OP_MOVEP, VAR(2), 0, VAR(3),
    // vm_push_eq(vm, agent1->ports[1], agent_S_1);
    OP_PUSH, R(1), VAR(2),

    // struct Agent* agent_F = agent_make(ID_F);
    OP_MKAGENT, VAR(4), ID_F,
    // agent_F->ports[0] = name_a;
    OP_MOVEP, VAR(4), 0, VAR(1),
    // agent_F->ports[1] = name_b;
    OP_MOVEP, VAR(4), 1, VAR(3),
    // vm_push_eq(vm, agent0->ports[0], agent_F);
    OP_PUSH, L(0), VAR(4),
    OP_RETURN};

// S(x) >< App(y, r) => r~F(x, y);
const uint8_t rule_s_app[] = {
    // struct Agent* agent_f = agent_make(ID_F);
    OP_MKAGENT, VAR(0), ID_F,
    // agent_f->ports[0] = agent0->ports[0];
    OP_MOVEP, VAR(0), 0, L(0),
    // agent_f->ports[1] = agent1->ports[0];
    OP_MOVEP, VAR(0), 1, R(0),
    // vm_push_eq(vm, agent1->ports[1], agent_f);
    OP_PUSH, R(1), VAR(0),
    OP_RETURN};

// S(x) >< Delta(y, z, r) => z~F(a, b), x~App(a, c), y~App(b, App(c, r));
const uint8_t rule_s_delta[] = {
    // struct Agent* agent_F = agent_make(ID_F);
    OP_MKAGENT, VAR(0), ID_F,
    // struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(2),
    // agent_F->ports[0] = name_a;
    OP_MOVEP, VAR(0), 0, VAR(1),
    // agent_F->ports[1] = name_b;
    OP_MOVEP, VAR(0), 1, VAR(2),
    // vm_push_eq(vm, agent1->ports[1], agent_F);
    OP_PUSH, R(1), VAR(0),

    // struct Agent* agent_app_0 = agent_make(ID_APP);
    OP_MKAGENT, VAR(3), ID_APP,
    // struct Agent* name_c = agent_make_name();
    OP_MKNAME, VAR(4),
    // agent_app_0->port[0] = name_a;
    OP_MOVEP, VAR(3), 0, VAR(1),
    // agent_app_0->port[1] = name_c;
    OP_MOVEP, VAR(3), 1, VAR(4),
    // vm_push_eq(vm, agent0->ports[0], agent_app_0);
    OP_PUSH, L(0), VAR(3),

    // struct Agent* agent_app_1 = agent_make(ID_APP);
    OP_MKAGENT, VAR(5), ID_APP,
    // agent_app_1->ports[0] = name_c;
    OP_MOVEP, VAR(5), 0, VAR(4),
    // agent_app_1->ports[1] = agent1->ports[2];
    OP_MOVEP, VAR(5), 1, R(2),

    // struct Agent* agent_app_2 = agent_make(ID_APP);
    OP_MKAGENT, VAR(6), ID_APP,
    // agent_app_2->ports[0] = name_b;
    OP_MOVEP, VAR(6), 0, VAR(2),
    // agent_app_2->ports[1] = agent_app_1;
    OP_MOVEP, VAR(6), 1, VAR(5),
    // vm_push_eq(vm, agent1->ports[0], agent_app_2);
    OP_PUSH, R(0), VAR(6),
    OP_RETURN};

// F(w, x) >< F(y, z) => w~F(a, c), x~F(d, b), y~F(d, a), z~F(b, c);
const uint8_t rule_f_f[] = {
    // struct Agent* agent_F_0 = agent_make(ID_F);
    OP_MKAGENT, VAR(0), ID_F,
    // struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(2),
    // agent_F_0->ports[0] = name_a;
    OP_MOVEP, VAR(0), 0, VAR(1),
    // agent_F_0->ports[1] = name_b;
    OP_MOVEP, VAR(0), 1, VAR(2),
    // vm_push_eq(vm, agent0->ports[0], agent_F_0);
    OP_PUSH, L(0), VAR(0),

    // struct Agent* agent_F_1 = agent_make(ID_F);
    OP_MKAGENT, VAR(3), ID_F,
    // struct Agent* name_c = agent_make_name();
    OP_MKNAME, VAR(4),
    // struct Agent* name_d = agent_make_name();
    OP_MKNAME, VAR(5),
    // agent_F_1->ports[0] = name_c;
    OP_MOVEP, VAR(3), 0, VAR(4),
    // agent_F_1->ports[1] = name_d;
    OP_MOVEP, VAR(3), 1, VAR(5),
    // vm_push_eq(vm, agent0->ports[1], agent_F_1);
    OP_PUSH, L(1), VAR(3),

    // struct Agent* agent_F_2 = agent_make(ID_F);
    OP_MKAGENT, VAR(6), ID_F,
    // agent_F_2->ports[0] = name_d;
    OP_MOVEP, VAR(3), 0, VAR(5),
    // agent_F_2->ports[1] = name_b;
    OP_MOVEP, VAR(3), 1, VAR(2),
    // vm_push_eq(vm, agent1->ports[1], agent_F_2);
    OP_PUSH, R(1), VAR(6),

    // struct Agent* agent_F_3 = agent_make(ID_F);
    OP_MKAGENT, VAR(7), ID_F,
    // agent_F_3->ports[0] = name_c;
    OP_MOVEP, VAR(7), 0, VAR(4),
    // agent_F_3->ports[1] = name_a;
    OP_MOVEP, VAR(7), 1, VAR(1),
    // vm_push_eq(vm, agent1->ports[0], agent_F_3);
    OP_PUSH, R(0), VAR(7),
    OP_RETURN};

// F(x, y) >< App(z, r) => x~Delta(y, z, r);
const uint8_t rule_f_app[] = {
    // struct Agent* agent_app1 = agent_make(ID_APP1);
    OP_MKAGENT, VAR(0), ID_DELTA,
    // agent_app1->ports[0] = agent0->ports[0];
    OP_MOVEP, VAR(0), 0, L(0),
    // agent_app1->ports[1] = agent0->ports[1];
    OP_MOVEP, VAR(0), 1, L(1),
    // agent_app1->ports[2] = agent1->ports[0];
    OP_MOVEP, VAR(0), 2, R(0),
    // vm_push_eq(vm, agent1->ports[1], agent_app1);
    OP_PUSH, R(1), VAR(0),
    OP_RETURN};

// F(w, x) >< Delta(y, z, r) => z~App(w, App(x, r)), y~K;
const uint8_t rule_f_delta[] = {
    // struct Agent* agent_app_0 = agent_make(ID_APP);
    OP_MKAGENT, VAR(0), ID_APP,
    // agent_app_0->ports[0] = agent0->ports[1];
    OP_MOVEP, VAR(0), 0, L(1),
    // agent_app_0->ports[1] = agent1->ports[2];
    OP_MOVEP, VAR(0), 1, R(2),

    // struct Agent* agent_app_1 = agent_make(ID_APP);
    OP_MKAGENT, VAR(1), ID_APP,
    // agent_app_1->ports[0] = agent0->ports[0];
    OP_MOVEP, VAR(1), 0, L(0),
    // agent_app_1->ports[1] = agent_app_0;
    OP_MOVEP, VAR(1), 1, VAR(0),
    // vm_push_eq(vm, agent_app_1, agent1->ports[1]);
    OP_PUSH, R(1), VAR(2),

    // struct Agent* agent_K = agent_make(ID_K);
    OP_MKAGENT, VAR(2), ID_K,
    // vm_push_eq(vm, agent_K, agent1->ports[0]);
    OP_PUSH, VAR(2), R(0),
    OP_RETURN};

const uint8_t* CodeTable[CODE_TABLE_SIZE] = {
    rule_k_k, rule_k_s, rule_k_f, rule_k_app, rule_k_delta,
              NULL,     rule_s_f, rule_s_app, rule_s_delta,
                        rule_f_f, rule_f_app, rule_f_delta,
                                  NULL,       NULL,
                                              NULL};

#endif