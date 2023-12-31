// Design and implementation of a low-level language for interaction nets
// https://sussex.figshare.com/articles/thesis/Design_and_implementation_of_a_low-level_language_for_interaction_nets/23417312

#include "vm.h"

struct VM {
    uint8_t word_size;

    struct EqStack* active_pairs; // Stack of Equations
    struct AgentHeap* heap;
    struct Agent* reg[MAX_REG_SIZE];
    
    struct Tape* tape;

    struct Agent* interface;
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
    vm->interface = NULL;
    return vm;
}

void vm_set_code(struct VM* vm, uint8_t* code) {
    if (vm->tape != NULL) {
        tape_free(vm->tape);
    }

    vm->tape = tape_make();
    tape_set(vm->tape, code);
}

// TODO use names instead of copy
// calculate left child, and connect it to the F node, and use a name instead of
// the right child
size_t _vm_code_from_program(struct ByteArray* byte_array, struct Program* prg,
    size_t next_id, size_t word_size)
{
    // The resulting tree should get copied to reg0.
    switch (program_get_type(prg)) {
        case PROGRAM_TYPE_LEAF: {
            byte_array_add_byte(byte_array, OP_MKAGENT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, ID_K);
            // Store it
            byte_array_add_byte(byte_array, OP_STORE);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_word(byte_array, next_id, word_size);
            return next_id;
        }
        case PROGRAM_TYPE_STEM: {
            // Create instructions for the subtree
            size_t child_addr =
                _vm_code_from_program(byte_array, program_get_child(prg, 0),
                    next_id, word_size);
            // Create the S agent
            byte_array_add_byte(byte_array, OP_MKAGENT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, ID_S);
            // Load the child
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child_addr, word_size);
            byte_array_add_byte(byte_array, 1);
            // Connect its aux port to reg1
            byte_array_add_byte(byte_array, OP_CONNECT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 1);
            // Store it
            byte_array_add_byte(byte_array, OP_STORE);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_word(byte_array, child_addr + 1, word_size);
            return child_addr + 1;
        }
        case PROGRAM_TYPE_FORK: {
            // Create the children
            size_t child0_addr =
                _vm_code_from_program(byte_array, program_get_child(prg, 0),
                    next_id, word_size);
            size_t child1_addr =
                _vm_code_from_program(byte_array, program_get_child(prg, 1),
                    child0_addr + 1, word_size);
            // Load the children
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child0_addr, word_size);
            byte_array_add_byte(byte_array, 1);
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child1_addr, word_size);
            byte_array_add_byte(byte_array, 2);
            // Create the F agent
            byte_array_add_byte(byte_array, OP_MKAGENT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, ID_F);
            // Connect its first aux port to reg1
            byte_array_add_byte(byte_array, OP_CONNECT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 1);
            // Connect its second aux port to reg2
            byte_array_add_byte(byte_array, OP_CONNECT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 1);
            byte_array_add_byte(byte_array, 2);
            // Store it
            byte_array_add_byte(byte_array, OP_STORE);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_word(byte_array, child1_addr + 1, word_size);
            return child1_addr + 1;
        }
    }
    return 0;
}

size_t _vm_code_from_tree(struct ByteArray* byte_array, struct Tree* tree,
    size_t next_id, size_t word_size)
{
    switch (tree_get_type(tree)) {
        case TREE_TYPE_PROGRAM: {
            struct Program* prg = tree_get_program(tree);
            return _vm_code_from_program(byte_array, prg, next_id, word_size);
        }
        case TREE_TYPE_APPLY: {
            struct Tree* subtree0 = tree_get_apply(tree, 0);
            struct Tree* subtree1 = tree_get_apply(tree, 1);
            // Create instructions for the second subtree
            size_t child0_addr = _vm_code_from_tree(byte_array, subtree0,
                next_id, word_size);
            size_t child1_addr = _vm_code_from_tree(byte_array, subtree1,
                child0_addr + 1, word_size);
            // Create the App agent
            byte_array_add_byte(byte_array, OP_MKAGENT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, ID_APP);
            // Load the children
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child0_addr, word_size);
            byte_array_add_byte(byte_array, 1);
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child1_addr, word_size);
            byte_array_add_byte(byte_array, 2);
            // Connect its first aux port to reg1
            byte_array_add_byte(byte_array, OP_CONNECT);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 2);
            // Create instructions for the first subtree
            // Push the result and the App agent to the stack
            byte_array_add_byte(byte_array, OP_PUSH);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_byte(byte_array, 1);
            // Store it
            byte_array_add_byte(byte_array, OP_STORE);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_word(byte_array, child1_addr + 1, word_size);
            return child1_addr + 1;
        }
    }
    return 0;
}

void vm_from_tree(struct VM* vm, struct Tree* tree) {
    struct ByteArray* byte_array = byte_array_make();

    // Convert a tree to a list of instructions
    size_t tree_addr = _vm_code_from_tree(byte_array, tree, 0, vm->word_size);
    byte_array_add_byte(byte_array, OP_LOAD);
    byte_array_add_word(byte_array, tree_addr, vm->word_size);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_byte(byte_array, OP_MKNAME);
    byte_array_add_byte(byte_array, 1);
    byte_array_add_byte(byte_array, OP_PUSH);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_byte(byte_array, 1);
    byte_array_add_byte(byte_array, OP_RETURN);

    // Set the code for the VM
    tape_set(vm->tape, byte_array_get_bytes(byte_array));

    printf("Print instructions\n");
    vm_print(vm);
    printf("Finish\n");

    // Execute the instructions
    _vm_exec(vm);

    // Free the code
    free_mem("vm_from_tree", vm->tape->code);
    tape_set(vm->tape, NULL);
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
    // First, we make sure that x <= y. If it's not true, we swap x and y.
    // Then we get the correct index by this formula:
    // y * n + x - (y * (y + 1)) / 2)
    if (agent0_type > agent1_type) {
        uint8_t temp = agent0_type;
        agent0_type = agent1_type;
        agent1_type = temp;
    }
    tape_set(vm->tape,
        CodeTable[agent1_type * (MAX_AGENT_ID - 1) + agent0_type -
            (agent1_type * (agent1_type + 1)) / 2]);

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
            break;
        }
        case OP_RETURN: {
            return;
        }
    }
    return;
}

struct Tree* vm_readback(struct VM* vm) {
    // TODO
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

void vm_print(struct VM* vm) {
    tape_print(vm->tape);
}
