// Design and implementation of a low-level language for interaction nets
// https://sussex.figshare.com/articles/thesis/Design_and_implementation_of_a_low-level_language_for_interaction_nets/23417312

#include "vm.h"

struct VM {
    uint8_t word_size;

    struct EqStack* active_pairs; // Stack of Equations
    struct AgentHeap* agent_heap;
    struct AgentHeap* name_heap;
    struct Agent* reg[MAX_REG_SIZE];
    
    struct Tape* tape;

    struct Agent* interface;
};

struct VM* vm_make(uint8_t word_size) {
    struct VM* vm = allocate_mem("vm_make", NULL, sizeof(struct VM));
    vm->word_size = word_size;
    vm->active_pairs = eq_stack_make(sizeof(struct Equation));
    vm->agent_heap = agent_heap_make();
    vm->name_heap = agent_heap_make();

    vm->tape = tape_make();
    vm->interface = NULL;
    return vm;
}

void vm_set_code(struct VM* vm, uint8_t* code) {
    if (vm->tape != NULL) {
        tape_free(vm->tape);
    }

    vm->tape = tape_make(code);
}

void vm_eval(struct VM* vm) {
    while (vm_step(vm) == EvalRunning);
}

uint8_t _vm_read_byte(struct VM* vm) {
    return tape_read_byte(vm->tape);
}

VM_WORD _vm_read_word(struct VM* vm) {
    return tape_read_word(vm->tape, vm->word_size);
}

// Evaluate by one step, return true if OP_RETURN is encountered, FALSE
// otherwise
enum EvalState vm_step(struct VM* vm) {
    // Pop an equation from the stack
    struct Equation eq = eq_stack_pop(vm->active_pairs);

    // Check if any of the agents are names
    uint8_t agent0_type = agent_get_type(eq.agent0);
    uint8_t agent1_type = agent_get_type(eq.agent1);
    BOOL continue_eval = FALSE;
    if (agent1_type != ID_NAME) {
        if (agent0_type != ID_NAME) {
            // Continue with evaluation
            continue_eval = TRUE;
        } else {
            // x = Alpha(x1, ..., xn)
            agent_set_port(eq.agent0, 0, eq.agent1);
        }
    } else {
        if (agent0_type != ID_NAME) {
            // Both are names
            agent_free(eq.agent0);
            agent_free(eq.agent1);
        } else {
            // Alpha(x1, ..., xn) = y and x = y
            agent_set_port(eq.agent1, 0, eq.agent0);
        }
    }

    if (continue_eval == FALSE) {
        return EvalRunning;
    }

    // Set up registers
    for (uint8_t i = 0; i < MAX_AUX_PORT_NUM; i++) {
        vm->reg[i] = agent_get_port(eq.agent0, i);
        vm->reg[MAX_AUX_PORT_NUM + i] = agent_get_port(eq.agent1, i);
    }
    // Defensive programming: null out all the other registers
    for (uint8_t i = 2 * MAX_AUX_PORT_NUM; i < MAX_REG_SIZE; i++) {
        vm->reg[i] = NULL;
    }

    // Load the instructions for this pair
    tape_set(vm->tape,
        CodeTable[agent_get_type(eq.agent0)][agent_get_type(eq.agent1)]);

    // Execute instructions until an OP_RETURN is encountered
    uint8_t current_instruction = _vm_read_byte(vm);
    while (current_instruction != OP_RETURN) {
        vm_exec_instruction(vm, current_instruction);
        current_instruction = _vm_read_byte(vm);
    }

    // Free the agents
    agent_free(eq.agent0);
    agent_free(eq.agent1);

    // Check if the equation stack is empty. Return EvalFinished if it is empty,
    // and EvalRunning otherwise
    if (eq_stack_size(vm->active_pairs) == 0) {
        return EvalFinished;
    } else {
        return EvalRunning;
    }
}

void vm_exec_instruction(struct VM* vm, uint8_t instruction) {
    switch (instruction) {
        case OP_MKAGENT: {
            uint8_t index = _vm_read_byte(vm);
            uint8_t type = _vm_read_byte(vm);
            uint8_t arity = Arities[type];
            vm->reg[index] = agent_make(type, arity);
            break;
        }
        case OP_MKNAME: {
            uint8_t index = _vm_read_byte(vm);
            vm->reg[index] = agent_make_name();
            break;
        }
        // case OP_MKIND: {
        //     uint8_t index = _vm_read_byte(vm);
        //     vm->reg[index] = agent_make_ind();
        //     break;
        // }
        // case OP_FREE: {
        //     uint8_t index = _vm_read_byte(vm);
        //     agent_free(vm->reg[index]);
        //     break;
        // }
        case OP_MOVEP: {
            // MOVEP A B C -> Reg[A]->port[B] = Reg[C]
            uint8_t src_agent_index = _vm_read_byte(vm);
            uint8_t port_num = _vm_read_byte(vm);
            uint8_t dst_agent_index = _vm_read_byte(vm);
            struct Agent* src_agent = vm->reg[src_agent_index];
            struct Agent* dst_agent = vm->reg[dst_agent_index];
            agent_set_port(src_agent, port_num, dst_agent);
            break;
        }
        // case OP_CHGID: {
        //     uint8_t index = _vm_read_byte(vm);
        //     uint8_t type = _vm_read_byte(vm);
        //     agent_set_type(vm->reg[index], type);
        //     break;
        // }
        case OP_PUSH: {
            uint8_t index0 = _vm_read_byte(vm);
            uint8_t index1 = _vm_read_byte(vm);
            struct Equation eq;
            eq.agent0 = vm->reg[index0];
            eq.agent1 = vm->reg[index1];
            eq_stack_push(vm->active_pairs, eq);
            break;
        }
        case OP_RETURN: {
            return;
        }
    }
    return;
}

void vm_free(struct VM* vm) {
    assert (vm != NULL);

    eq_stack_free(vm->active_pairs);
    agent_heap_free(vm->agent_heap);
    agent_heap_free(vm->name_heap);
    free_mem("vm_free", vm);
}
