#include "code.h"

#include <stdint.h>

#include "global.h"

// char Symbols[MAX_AGENT_ID] = {'K', 'S', 'F', '@', '#'};
const uint8_t Arities[MAX_AGENT_ID] = {0, 1, 2, 2, 3};

// K >< E => ;
const uint8_t rule_k_e[] = {OP_RETURN};

// K >< D(x, y) => K~x, K~y;
const uint8_t rule_k_d[] = {
    // struct Agent* agent_K_0 = agent_make(ID_K);
    OP_MKAGENT, VAR(0), ID_K,
    // struct Agent* agent_K_1 = agent_make(ID_K);
    OP_MKAGENT, VAR(1), ID_K,
    // vm_push_eq(vm, agent_K_0, agent1->ports[0]);
    OP_PUSH, VAR(0), R(0),
    // vm_push_eq(vm, agent_K_1, agent1->ports[1]);
    OP_PUSH, VAR(1), R(1),
    OP_RETURN};

// K >< A(x, r) => S(x)~r;
const uint8_t rule_k_a[] = {
    // struct Agent* agent_S = agent_make(ID_S);
    OP_MKAGENT, VAR(0), ID_S,
    // agent_S->ports[0] = agent1->ports[0];
    OP_CONNECT, VAR(0), 0, R(0),
    // vm_push_eq(vm, agent_S, agent1->ports[1]);
    OP_PUSH, VAR(0), R(1),
    OP_RETURN};

// K >< T(y, z, r) => y~r, z~E;
const uint8_t rule_k_t[] = {
    // vm_push_eq(vm, agent1->ports[0], agent1->ports[2]);
    OP_MKAGENT, VAR(0), ID_E,
    OP_PUSH, R(0), R(2),
    OP_PUSH, R(1), VAR(0), // TODO maybe add a FREE_REC to recursively free the agent R(1)
    OP_RETURN};

// S(x) >< E => x~E;
const uint8_t rule_s_e[] = {
    // vm_push_eq(vm, agent0, agent1->ports[0]);
    OP_MKAGENT, VAR(0), ID_E,
    OP_PUSH, L(0), VAR(0),
    OP_RETURN};

// S(x) >< D(y, z) => x~D(a, b), S(a)~y, S(b)~z;
const uint8_t rule_s_d[] = {
    // struct Agent* agent_D = agent_make(ID_D);
    OP_MKAGENT, VAR(0), ID_D,
    // struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(2),
    // agent_D->ports[0] = name_a;
    OP_CONNECT, VAR(0), 0, VAR(1),
    // agent_D->ports[1] = name_b;
    OP_CONNECT, VAR(0), 1, VAR(2),
    // vm_push_eq(vm, agent0->ports[0], agent_D);
    OP_PUSH, L(0), VAR(0),
    
    // struct Agent* agent_S_0 = agent_make(ID_S);
    OP_MKAGENT, VAR(3), ID_S,
    // agent_S_0->ports[0] = name_a;
    OP_CONNECT, VAR(3), 0, VAR(1),
    // vm_push_eq(vm, agent_S_0, agent1->ports[0]);
    OP_PUSH, VAR(3), R(0),

    // struct Agent* agent_S_1 = agent_make(ID_S);
    OP_MKAGENT, VAR(4), ID_S,
    // agent_S_1->ports[0] = name_b;
    OP_CONNECT, VAR(4), 0, VAR(2),
    // vm_push_eq(vm, agent_S_1, agent1->ports[1]);
    OP_PUSH, VAR(4), R(1),
    OP_RETURN};

// S(x) >< A(y, r) => F(x, y)~r;
const uint8_t rule_s_a[] = {
    // struct Agent* agent_F = agent_make(ID_F);
    OP_MKAGENT, VAR(0), ID_F,
    // agent_F->ports[0] = agent0->ports[0];
    OP_CONNECT, VAR(0), 0, L(0),
    // agent_F->ports[1] = agent1->ports[0];
    OP_CONNECT, VAR(0), 1, R(0),
    // vm_push_eq(vm, agent_F, agent1->ports[1]);
    OP_PUSH, VAR(0), R(1),
    OP_RETURN};

// S(x) >< T(y, z, r) => z~D(a, b), x~A(a, c), y~A(b, A(c, r));
const uint8_t rule_s_t[] = {
    // struct Agent* agent_D = agent_make(ID_D);
    OP_MKAGENT, VAR(0), ID_D,
    // struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(2),
    // agent_D->ports[0] = name_a;
    OP_CONNECT, VAR(0), 0, VAR(1),
    // agent_D->ports[1] = name_b;
    OP_CONNECT, VAR(0), 1, VAR(2),
    // vm_push_eq(vm, agent1->ports[1], agent_D);
    OP_PUSH, R(1), VAR(0),

    // struct Agent* agent_A0 = agent_make(ID_A);
    OP_MKAGENT, VAR(3), ID_A,
    // struct Agent* name_c = agent_make_name();
    OP_MKNAME, VAR(4),
    // agent_A0->port[0] = name_a;
    OP_CONNECT, VAR(3), 0, VAR(1),
    // agent_A0->port[1] = name_c;
    OP_CONNECT, VAR(3), 1, VAR(4),
    // vm_push_eq(vm, agent0->ports[0], agent_A0);
    OP_PUSH, L(0), VAR(3),

    // struct Agent* agent_A1 = agent_make(ID_A);
    OP_MKAGENT, VAR(5), ID_A,
    // agent_A1->ports[0] = name_c;
    OP_CONNECT, VAR(5), 0, VAR(4),
    // agent_A1->ports[1] = agent1->ports[2];
    OP_CONNECT, VAR(5), 1, R(2),

    // struct Agent* agent_A2 = agent_make(ID_A);
    OP_MKAGENT, VAR(6), ID_A,
    // agent_A2->ports[0] = name_b;
    OP_CONNECT, VAR(6), 0, VAR(2),
    // agent_A2->ports[1] = agent_app_1;
    OP_CONNECT, VAR(6), 1, VAR(5),
    // vm_push_eq(vm, agent1->ports[0], agent_A2);
    OP_PUSH, R(0), VAR(6),
    OP_RETURN};

// F(x, y) >< E => x~E, y~E;
const uint8_t rule_f_e[] = {
    // vm_push_eq(vm, agent0, agent1->ports[0]);
    OP_MKAGENT, VAR(0), ID_E,
    OP_PUSH, L(0), VAR(0),
    OP_MKAGENT, VAR(1), ID_E,
    OP_PUSH, L(1), VAR(1),
    OP_RETURN};

// F(w, x) >< D(y, z) => F(a, b)~z, F(c, d)~y, w~D(c, a), x~D(d, b);
const uint8_t rule_f_d[] = {
    // 0 struct Agent* agent_F0 = agent_make(ID_F);
    OP_MKAGENT, VAR(0), ID_F,
    // 3 struct Agent* name_a = agent_make_name();
    OP_MKNAME, VAR(1),
    // 5 struct Agent* name_b = agent_make_name();
    OP_MKNAME, VAR(2),
    // 7 agent_F0->ports[0] = name_a;
    OP_CONNECT, VAR(0), 0, VAR(1),
    // 11 agent_F0->ports[1] = name_b;
    OP_CONNECT, VAR(0), 1, VAR(2),
    // 15 vm_push_eq(vm, agent_F0, agent1->ports[1]);
    OP_PUSH, VAR(0), R(1),

    // 18 struct Agent* agent_F1 = agent_make(ID_F);
    OP_MKAGENT, VAR(3), ID_F,
    // 21 struct Agent* name_c = agent_make_name();
    OP_MKNAME, VAR(4),
    // 23 struct Agent* name_d = agent_make_name();
    OP_MKNAME, VAR(5),
    // 25 agent_F1->ports[0] = name_c;
    OP_CONNECT, VAR(3), 0, VAR(4),
    // 29 agent_F1->ports[1] = name_d;
    OP_CONNECT, VAR(3), 1, VAR(5),
    // 33 vm_push_eq(vm, agent_F1, agent1->ports[0]);
    OP_PUSH, VAR(3), R(0),

    // 36 struct Agent* agent_D0 = agent_make(ID_D);
    OP_MKAGENT, VAR(6), ID_D,
    // 39 agent_D0->ports[0] = name_c;
    OP_CONNECT, VAR(6), 0, VAR(4),
    // 43 agent_D0->ports[1] = name_a;
    OP_CONNECT, VAR(6), 1, VAR(1),
    // 47 vm_push_eq(vm, agent0->ports[0], agent_D0);
    OP_PUSH, L(0), VAR(6),

    // 50 struct Agent* agent_D1 = agent_make(ID_F);
    OP_MKAGENT, VAR(7), ID_D,
    // 53 agent_D1->ports[0] = name_d;
    OP_CONNECT, VAR(7), 0, VAR(5),
    // 57 agent_D1->ports[1] = name_b;
    OP_CONNECT, VAR(7), 1, VAR(2),
    // 61 vm_push_eq(vm, agent0->ports[1], agent_D1);
    OP_PUSH, L(1), VAR(7),
    OP_RETURN};

// F(x, y) >< A(z, r) => x~T(y, z, r);
const uint8_t rule_f_a[] = {
    // struct Agent* agent_T = agent_make(ID_T);
    OP_MKAGENT, VAR(0), ID_T,
    // agent_T->ports[0] = agent0->ports[1];
    OP_CONNECT, VAR(0), 0, L(1),
    // agent_T->ports[1] = agent1->ports[0];
    OP_CONNECT, VAR(0), 1, R(0),
    // agent_T->ports[2] = agent1->ports[1];
    OP_CONNECT, VAR(0), 2, R(1),
    // vm_push_eq(vm, agent0->ports[0], agent_T);
    OP_PUSH, L(0), VAR(0),
    OP_RETURN};

// F(w, x) >< T(y, z, r) => z~A(w, A(x, r)), y~E;
const uint8_t rule_f_t[] = {
    // struct Agent* agent_A0 = agent_make(ID_A);
    OP_MKAGENT, VAR(0), ID_A,
    // agent_A0->ports[0] = agent0->ports[1];
    OP_CONNECT, VAR(0), 0, L(1),
    // agent_A0->ports[1] = agent1->ports[2];
    OP_CONNECT, VAR(0), 1, R(2),

    // struct Agent* agent_A1 = agent_make(ID_A);
    OP_MKAGENT, VAR(1), ID_A,
    // agent_A1->ports[0] = agent0->ports[0];
    OP_CONNECT, VAR(1), 0, L(0),
    // agent_A1->ports[1] = agent_A0;
    OP_CONNECT, VAR(1), 1, VAR(0),
    // vm_push_eq(vm, agent1->ports[1], agent_A1);
    OP_PUSH, R(1), VAR(1),

    // struct Agent* agent_E = agent_make(ID_E);
    OP_MKAGENT, VAR(2), ID_E,
    // vm_push_eq(vm, agent1->ports[0], agent_E);
    OP_PUSH, R(0), VAR(2),
    OP_RETURN};

const uint8_t* CodeTable[CODE_TABLE_SIZE] = {
    rule_k_e, rule_k_d, rule_k_a, rule_k_t,
    rule_s_e, rule_s_d, rule_s_a, rule_s_t,
    rule_f_e, rule_f_d, rule_f_a, rule_f_t};

// TODO Only for debug - put it in a debug-only define-block
const char* CodeNameTable[CODE_TABLE_SIZE] = {
    "rule_k_e", "rule_k_d", "rule_k_a", "rule_k_t",
    "rule_s_e", "rule_s_d", "rule_s_a", "rule_s_t",
    "rule_f_e", "rule_f_d", "rule_f_a", "rule_f_t"};