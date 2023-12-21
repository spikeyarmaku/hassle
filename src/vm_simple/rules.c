#include "rules.h"

void rule_table_init() {
    for (uint8_t i = 0; i < MAX_AGENT_ID; i++) {
        for (uint8_t j = 0; j < MAX_AGENT_ID; j++) {
            RuleTable[i][j] = &missing_rule;
        }
    }

    RuleTable[ID_K][ID_APP]     = &rule_K_App;
    RuleTable[ID_S][ID_APP]     = &rule_S_App;
    RuleTable[ID_F][ID_APP]     = &rule_F_App;
    RuleTable[ID_K][ID_APP1]    = &rule_K_App1;
    RuleTable[ID_S][ID_APP1]    = &rule_S_App1;
    RuleTable[ID_F][ID_APP1]    = &rule_F_App1;
    RuleTable[ID_K][ID_F]       = &rule_K_F;
    RuleTable[ID_S][ID_F]       = &rule_S_F;
    RuleTable[ID_F][ID_F]       = &rule_F_F;
    RuleTable[ID_K][ID_K]       = &rule_K_K;
    RuleTable[ID_K][ID_S]       = &rule_K_S;
}

void missing_rule(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    printf("Error: missing rule %d - %d\n", agent0->type, agent1->type);
}

void rule_K_App(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // K = @(x, r) => r = S(x)
    struct Agent* agent_s = agent_make(ID_S);
    agent_s->ports[0] = agent1->ports[0];
    vm_push_eq(vm, agent1->ports[1], agent_s);
    
    agent_free(agent0);
    agent_free(agent1);
}

void rule_S_App(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // S(x) = @(y, r) => r = F(x, y)
    struct Agent* agent_f = agent_make(ID_F);
    agent_f->ports[0] = agent0->ports[0];
    agent_f->ports[1] = agent1->ports[0];
    vm_push_eq(vm, agent1->ports[1], agent_f);
    
    agent_free(agent0);
    agent_free(agent1);
}

void rule_F_App(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // F(x, y) = @(z, r) => r = #(x, y, z)
    struct Agent* agent_app1 = agent_make(ID_APP1);
    agent_app1->ports[0] = agent0->ports[0];
    agent_app1->ports[1] = agent0->ports[1];
    agent_app1->ports[2] = agent1->ports[0];
    vm_push_eq(vm, agent1->ports[1], agent_app1);
    
    agent_free(agent0);
    agent_free(agent1);
}

void rule_K_App1(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // K = #(y, z, r) => y = r, K = z
    vm_push_eq(vm, agent1->ports[0], agent1->ports[2]);
    agent_free_rec(agent1->ports[1]);
    
    agent_free(agent0);
    agent_free(agent1);
}

void rule_S_App1(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // S(x) = #(y, z, r) => @(b, r) = a, @(c, a) = y, @(d, b) = x, F(d, c) = z
    struct Agent* agent_app_0 = agent_make(ID_APP);
    struct Agent* name_a = agent_make_name();
    struct Agent* name_b = agent_make_name();
    agent_app_0->ports[0] = name_b;
    agent_app_0->ports[1] = agent1->ports[2];
    vm_push_eq(vm, agent_app_0, name_a);

    struct Agent* agent_app_1 = agent_make(ID_APP);
    struct Agent* name_c = agent_make_name();
    agent_app_1->ports[0] = name_c;
    agent_app_1->ports[1] = name_a;
    vm_push_eq(vm, agent_app_1, agent1->ports[0]);

    struct Agent* agent_app_2 = agent_make(ID_APP);
    struct Agent* name_d = agent_make_name();
    agent_app_2->ports[0] = name_d;
    agent_app_2->ports[1] = name_b;
    vm_push_eq(vm, agent_app_2, agent0->ports[0]);

    struct Agent* agent_F = agent_make(ID_F);
    agent_F->ports[0] = name_d;
    agent_F->ports[1] = name_c;
    vm_push_eq(vm, agent_F, agent1->ports[1]);

    agent_free(agent0);
    agent_free(agent1);
}

void rule_F_App1(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // F(w, x) = #(y, z, r) => @(x, r) = a, @(w, a) = z
    struct Agent* agent_app_1 = agent_make(ID_APP);
    struct Agent* name_a = agent_make_name();
    agent_app_1->ports[0] = agent0->ports[1];
    agent_app_1->ports[1] = agent1->ports[2];
    vm_push_eq(vm, agent_app_1, name_a);

    struct Agent* agent_app_2 = agent_make(ID_APP);
    agent_app_2->ports[0] = agent0->ports[0];
    agent_app_2->ports[1] = name_a;
    vm_push_eq(vm, agent_app_2, agent1->ports[1]);
    
    agent_free(agent0);
    agent_free(agent1);
}

void rule_K_F(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // K = F(x, y) => K = x, K = y
    struct Agent* agent_K_0 = agent_make(ID_K);
    struct Agent* agent_K_1 = agent_make(ID_K);
    vm_push_eq(vm, agent_K_0, agent1->ports[0]);
    vm_push_eq(vm, agent_K_1, agent1->ports[1]);

    agent_free(agent0);
    agent_free(agent1);
}

void rule_S_F(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // S(z) = F(x, y) => S(a) = x, S(b) = y, F(a, b) = z
    struct Agent* agent_S_0 = agent_make(ID_S);
    struct Agent* name_a = agent_make_name();
    agent_S_0->ports[0] = name_a;
    vm_push_eq(vm, agent_S_0, agent1->ports[0]);

    struct Agent* agent_S_1 = agent_make(ID_S);
    struct Agent* name_b = agent_make_name();
    agent_S_1->ports[0] = name_b;
    vm_push_eq(vm, agent_S_1, agent1->ports[1]);

    struct Agent* agent_F = agent_make(ID_F);
    agent_F->ports[0] = name_a;
    agent_F->ports[1] = name_b;
    vm_push_eq(vm, agent_F, agent0->ports[0]);

    agent_free(agent0);
    agent_free(agent1);
}

void rule_F_F(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // F(w, x) = F(y, z) => F(a, b) = w, F(c, d) = x, F(d, b) = z, F(c, a) = y
    struct Agent* agent_F_0 = agent_make(ID_F);
    struct Agent* name_a = agent_make_name();
    struct Agent* name_b = agent_make_name();
    agent_F_0->ports[0] = name_a;
    agent_F_0->ports[1] = name_b;
    vm_push_eq(vm, agent_F_0, agent0->ports[0]);

    struct Agent* agent_F_1 = agent_make(ID_F);
    struct Agent* name_c = agent_make_name();
    struct Agent* name_d = agent_make_name();
    agent_F_1->ports[0] = name_c;
    agent_F_1->ports[1] = name_d;
    vm_push_eq(vm, agent_F_1, agent0->ports[1]);

    struct Agent* agent_F_2 = agent_make(ID_F);
    agent_F_2->ports[0] = name_d;
    agent_F_2->ports[1] = name_b;
    vm_push_eq(vm, agent_F_2, agent1->ports[1]);

    struct Agent* agent_F_3 = agent_make(ID_F);
    agent_F_3->ports[0] = name_c;
    agent_F_3->ports[1] = name_a;
    vm_push_eq(vm, agent_F_3, agent1->ports[0]);

    agent_free(agent0);
    agent_free(agent1);
}

void rule_K_K(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // K = K => [empty]
    agent_free(agent0);
    agent_free(agent1);
}

void rule_K_S(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    // K = S(x) = K = x
    vm_push_eq(vm, agent0, agent1->ports[0]);

    agent_free(agent0);
    agent_free(agent1);
}
