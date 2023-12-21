#include "eval.h"

void eval(struct VM* vm) {
    while (eval_step(vm) == TRUE);
}

BOOL eval_step(struct VM* vm) {
    struct Equation eq = eq_stack_pop(vm->eq_stack);
    if (eq.left == NULL) {
        return FALSE;
    }

    if (eq.right->type != ID_NAME) {
        if (eq.left->type != ID_NAME) {
            RuleTable[eq.left->type][eq.right->type](vm, eq.left, eq.right);
        } else {
            if (eq.left->ports[0] != NULL) {
                struct Agent* a1p0 = eq.left->ports[0];
                agent_free(eq.left);
                vm_push_eq(vm, a1p0, eq.right);
            } else {
                eq.left->ports[0] = eq.right;
            }
        }
    } else {
        if (eq.right->ports[0] != NULL) {
            struct Agent* a2p0 = eq.right->ports[0];
            agent_free(eq.right);
            vm_push_eq(vm, eq.left, a2p0);
        } else {
            eq.right->ports[0] = eq.left;
        }
    }
    return TRUE;
}
