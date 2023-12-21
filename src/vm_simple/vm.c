#include "vm.h"

struct VM* vm_make() {
    struct VM* vm = allocate_mem("vm_make", NULL, sizeof(struct VM));
    vm->eq_stack = eq_stack_make();
    vm->root_agent = NULL;
    return vm;
}

void vm_push_eq(struct VM* vm, struct Agent* agent0, struct Agent* agent1) {
    struct Equation eq;
    eq.left = agent0;
    eq.right = agent1;
    eq_stack_push(vm->eq_stack, eq);
}

void vm_free(struct VM* vm) {
    eq_stack_free(vm->eq_stack);
    agent_free_rec(vm->root_agent);
    free_mem("vm_free", vm);
}
