// Design and implementation of a low-level language for interaction nets
// https://sussex.figshare.com/articles/thesis/Design_and_implementation_of_a_low-level_language_for_interaction_nets/23417312

#include "vm.h"

#include "readback.h"
#include "convert.h"

struct VM {
    uint8_t word_size;

    struct EqStack* active_pairs; // Stack of Equations
    struct AgentHeap* heap;
    struct Agent* reg[MAX_REG_SIZE];
    
    struct Tape* tape;

    // struct Agent* interface;
};

size_t  _vm_code_from_program
    (struct ByteArray*, struct Program*, size_t, size_t);
size_t  _vm_code_from_tree
    (struct ByteArray*, struct Tree*, size_t, size_t);
void    _vm_exec_instruction    (struct VM*, uint8_t);
void    _vm_exec                (struct VM* vm);

struct VM* vm_make(uint8_t word_size) {
    struct VM* vm = allocate_mem("vm_make", NULL, sizeof(struct VM));
    vm->word_size = word_size;
    vm->active_pairs = eq_stack_make(sizeof(struct Equation));
    vm->heap = agent_heap_make();
    // vm->name_heap = agent_heap_make();

    vm->tape = tape_make();
    // vm->interface = NULL;
    return vm;
}

void vm_set_code(struct VM* vm, uint8_t* code) {
    if (vm->tape != NULL) {
        tape_free(vm->tape);
    }

    vm->tape = tape_make();
    tape_set(vm->tape, code);
}

void vm_from_tree(struct VM* vm, struct Tree* tree) {
    struct ByteArray* byte_array = convert_from_tree(tree, vm->word_size);

    // Set the code for the VM
    tape_set(vm->tape, byte_array_get_bytes(byte_array));

    // printf("Print instructions\n");
    // vm_print(vm);
    printf("Finish\n");

    // Execute the instructions
    _vm_exec(vm);

    // DEBUG PRINT
    // struct Tree* debug = readback(vm);
    // (void)debug;
    // tree_print(debug);

    // Free the code
    free_mem("vm_from_tree", vm->tape->code);
    tape_set(vm->tape, NULL);
}

void vm_eval(struct VM* vm) {
    // readback(vm);
    size_t step_number = 0;
    while (vm_step(vm) == EvalRunning) {
        // readback(vm);
        // printf("--\n");
        step_number++;
        if (step_number % 10000 == 0) {
            printf("Step %llu\n", step_number);
        }
    }
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
    if (eq_stack_size(vm->active_pairs) == 0) {
        return EvalFinished;
    }
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
            if (agent_get_port(eq.agent0, 0) == NULL) {
                // x is a name
                // x = Alpha(x1, ..., xn)
                agent_set_port(eq.agent0, 0, eq.agent1);
            } else {
                // x is an indirection
                struct Equation new_eq;
                new_eq.agent0 = agent_get_port(eq.agent0, 0);
                new_eq.agent1 = eq.agent1;
                eq_stack_push(vm->active_pairs, new_eq);
                agent_free(eq.agent0);
            }
        }
    } else {
        if (agent_get_port(eq.agent1, 0) == NULL) {
            // y is a name
            agent_set_port(eq.agent1, 0, eq.agent0);
        } else {
            // y is an indirection
            // Alpha(x1, ..., xn) = y and x = y
            struct Equation new_eq;
            new_eq.agent0 = eq.agent0;
            new_eq.agent1 = agent_get_port(eq.agent1, 0);
            eq_stack_push(vm->active_pairs, new_eq);
            agent_free(eq.agent1);
        }
    }

    if (continue_eval == FALSE) {
        return EvalRunning;
    }

    assert(agent0_type < agent1_type);

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
    uint8_t code_index = (agent0_type - ID_K) * 4 + (agent1_type - ID_E);
    tape_set(vm->tape, CodeTable[code_index]);
    // assert(CodeNameTable[code_index] != NULL);
    if (CodeNameTable[code_index] == NULL) {
        printf("ERROR: no rule found for %d - %d\n", agent0_type, agent1_type);
    // } else {
    //     printf("Executing reduction rule %s\n", CodeNameTable[code_index]);
    }

    // Execute instructions until an OP_RETURN is encountered
    _vm_exec(vm);

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

// Take and execute instructions until OP_RETURN is encountered
void _vm_exec(struct VM* vm) {
    uint8_t current_instruction = _vm_read_byte(vm);
    while (current_instruction != OP_RETURN) {
        _vm_exec_instruction(vm, current_instruction);
        current_instruction = _vm_read_byte(vm);
    }
}

void _vm_exec_instruction(struct VM* vm, uint8_t instruction) {
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
        case OP_CONNECT: {
            // MOVEP A B C -> Reg[A]->port[B] = Reg[C]
            uint8_t src_agent_index = _vm_read_byte(vm);
            uint8_t port_num = _vm_read_byte(vm);
            uint8_t dst_agent_index = _vm_read_byte(vm);
            struct Agent* src_agent = vm->reg[src_agent_index];
            struct Agent* dst_agent = vm->reg[dst_agent_index];
            agent_set_port(src_agent, port_num, dst_agent);
            break;
        }
        case OP_PUSH: {
            uint8_t index0 = _vm_read_byte(vm);
            uint8_t index1 = _vm_read_byte(vm);
            struct Equation eq;
            eq.agent0 = vm->reg[index0];
            eq.agent1 = vm->reg[index1];
            eq_stack_push(vm->active_pairs, eq);
            break;
        }
        case OP_STORE: {
            uint8_t reg_index = _vm_read_byte(vm);
            size_t g_index = _vm_read_word(vm);
            if (agent_heap_get_count(vm->heap) > g_index) {
                agent_heap_set(vm->heap, g_index, vm->reg[reg_index]);
            } else {
                agent_heap_add(vm->heap, vm->reg[reg_index]);
            }
            break;
        }
        case OP_LOAD: {
            size_t g_index = _vm_read_word(vm);
            uint8_t reg_index = _vm_read_byte(vm);
            vm->reg[reg_index] = agent_heap_get(vm->heap, g_index);
            break;
        }
        case OP_RETURN: {
            return;
        }
    }
    return;
}

struct Agent* _vm_get_pair(struct VM* vm, struct Agent* agent) {
    size_t pair_count = eq_stack_size(vm->active_pairs);
    for (size_t i = 0; i < pair_count; i++) {
         struct Equation eq = eq_stack_peek(vm->active_pairs, i);
        if (eq.agent0 == agent) {
            return eq.agent1;
        } else {
            if (eq.agent1 == agent) {
                return eq.agent0;
            }
        }
    }
    return NULL;
}

void vm_free(struct VM* vm) {
    assert (vm != NULL);

    eq_stack_free(vm->active_pairs);
    agent_heap_free(vm->heap);
    free_mem("vm_free", vm);
}

void vm_reset(struct VM* vm) {
    // TODO
}

struct EqStack* vm_get_active_pairs (struct VM* vm) {
    return vm->active_pairs;
}

struct AgentHeap* vm_get_heap(struct VM* vm) {
    return vm->heap;
}

void vm_print(struct VM* vm) {
    tape_print(vm->tape);
}

struct Agent* vm_get_interface(struct VM* vm) {
    return agent_heap_get(vm_get_heap(vm), 0);
}