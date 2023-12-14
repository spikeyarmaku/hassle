// Design and implementation of a low-level language for interaction nets
// https://sussex.figshare.com/articles/thesis/Design_and_implementation_of_a_low-level_language_for_interaction_nets/23417312

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
*/


#include "vm.h"

enum {OP_MKAGENT, OP_MKNAME, OP_MKIND, OP_FREE, OP_MOVEP, OP_MOVEG, OP_CHGID,
    OP_PUSH, OP_RETURN};

struct Equation {
    struct Agent* agent1;
    struct Agent* agent2;
};

struct VM {
    uint8_t word_size;
    struct BytePool* arities;

    // Active pairs
    struct ByteStack* active_pairs; // Stack of Equations
    // Each Register Reg(n) is used for each variable in LL0 and the Global
    // array G is used as the interface array.
    struct PtrPool* reg; // Pool of Agents
    // Interface - do we need it?
    struct PtrPool* global; // Pool of Agents
};

struct VM* vm_make(uint8_t word_size) {
    struct VM* vm = allocate_mem("vm_make", NULL, sizeof(struct VM));
    vm->word_size = word_size;
    vm->arities = bytepool_make(1);
    vm->active_pairs = bytestack_make(sizeof(struct Equation));
    vm->reg = ptrpool_make();
    vm->global = ptrpool_make();

    vm_add_agent_type(vm, 0); // Name type
    vm_add_agent_type(vm, 1); // Indirection type
    return vm;
}

void vm_add_agent_type(struct VM* vm, uint8_t arity) {
    bytepool_add_elem(vm->arities, &arity);
}

// Evaluate by one step, return true if OP_RETURN is encountered, FALSE
// otherwise
BOOL vm_step_code(struct VM* vm, struct Code* code) {
    uint8_t current_instruction = code_read_byte(code);
    switch (current_instruction) {
        case OP_MKAGENT: {
            VM_WORD index = code_read_word(code, vm->word_size);
            uint8_t type = code_read_byte(code);
            uint8_t arity = bytepool_get_byte(vm->arities, type);
            ptrpool_set(vm->reg, index, agent_make(type, arity));
            break;
        }
        case OP_MKNAME: {
            VM_WORD index = code_read_word(code, vm->word_size);
            uint8_t arity = bytepool_get_byte(vm->arities, 0);
            break;
        }
        case OP_MKIND: {
            VM_WORD index = code_read_word(code, vm->word_size);
            uint8_t arity = bytepool_get_byte(vm->arities, 1);
            break;
        }
        case OP_FREE: {
            VM_WORD index = code_read_word(code, vm->word_size);
            agent_free(ptrpool_get(vm->reg, index));
            break;
        }
        case OP_MOVEP: {
            // MOVEP A B C -> Reg[A]->port[B] = Reg[C]
            VM_WORD src_agent_index = code_read_word(code, vm->word_size);
            VM_WORD port_num = code_read_byte(code);
            VM_WORD dst_agent_index = code_read_word(code, vm->word_size);
            struct Agent* src_agent = ptrpool_get(vm->reg, src_agent_index);
            struct Agent* dst_agent = ptrpool_get(vm->reg, dst_agent_index);
            agent_set_port(src_agent, port_num, dst_agent);
            break;
        }
        case OP_MOVEG: {
            // TODO
            break;
        }
        case OP_CHGID: {
            VM_WORD agent_index = code_read_word(code, vm->word_size);
            VM_WORD type = code_read_byte(code);
            struct Agent* agent = ptrpool_get(vm->reg, agent_index);
            agent_set_type(agent, type);
            break;
        }
        case OP_PUSH: {
            VM_WORD agent1_index = code_read_word(code, vm->word_size);
            VM_WORD agent2_index = code_read_word(code, vm->word_size);
            struct Agent* agent1 = ptrpool_get(vm->reg, agent1_index);
            struct Agent* agent2 = ptrpool_get(vm->reg, agent2_index);
            struct Equation eq;
            eq.agent1 = agent1;
            eq.agent2 = agent2;
            bytestack_push(vm->active_pairs, &eq);
            break;
        }
        case OP_RETURN: {
            return TRUE;
        }
    }
    return FALSE;
}

void vm_add_pair(struct VM* vm, struct Agent* agent1, struct Agent* agent2) {
    struct Equation eq;
    eq.agent1 = agent1;
    eq.agent2 = agent2;
    bytestack_push(vm->active_pairs, &eq);
}
