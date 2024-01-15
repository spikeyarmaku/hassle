// Design and implementation of a low-level language for interaction nets
// https://sussex.figshare.com/articles/thesis/Design_and_implementation_of_a_low-level_language_for_interaction_nets/23417312

#include "vm.h"

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
            byte_array_add_byte(byte_array, ID_A);
            // Load the children
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child0_addr, word_size);
            byte_array_add_byte(byte_array, 1);
            byte_array_add_byte(byte_array, OP_LOAD);
            byte_array_add_word(byte_array, child1_addr, word_size);
            byte_array_add_byte(byte_array, 2);

            if (tree_get_type(subtree1) == TREE_TYPE_PROGRAM) {
                // If its right child is a program, connect its first aux port to
                // reg1
                byte_array_add_byte(byte_array, OP_CONNECT);
                byte_array_add_byte(byte_array, 0);
                byte_array_add_byte(byte_array, 0);
                byte_array_add_byte(byte_array, 2);
            } else {
                // Otherwise, create a name, and connect both the app and the
                // child to the name

                // Create a name
                byte_array_add_byte(byte_array, OP_MKNAME);
                byte_array_add_byte(byte_array, 3);

                // Connect the parent app's p0 to the name
                byte_array_add_byte(byte_array, OP_CONNECT);
                byte_array_add_byte(byte_array, 0);
                byte_array_add_byte(byte_array, 0);
                byte_array_add_byte(byte_array, 3);

                // Connect the child app's p1 to the name
                byte_array_add_byte(byte_array, OP_CONNECT);
                byte_array_add_byte(byte_array, 2);
                byte_array_add_byte(byte_array, 1);
                byte_array_add_byte(byte_array, 3);
            }
            // Create instructions for the first subtree
            // Push the result and the App agent to the stack
            byte_array_add_byte(byte_array, OP_PUSH);
            byte_array_add_byte(byte_array, 1);
            byte_array_add_byte(byte_array, 0);
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

    // Create the interface
    byte_array_add_byte(byte_array, OP_MKNAME);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_byte(byte_array, OP_STORE);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_word(byte_array, 0, vm->word_size);
    // Convert the tree to a list of instructions
    size_t tree_addr = _vm_code_from_tree(byte_array, tree, 1, vm->word_size);
    if (tree_get_type(tree) == TREE_TYPE_APPLY) {
        // If the tree is an application, connect its p1 to the interface

        // Load the interface
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, 0, vm->word_size);
        byte_array_add_byte(byte_array, 0);

        // Load the tree
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, tree_addr, vm->word_size);
        byte_array_add_byte(byte_array, 1);

        // Connect
        byte_array_add_byte(byte_array, OP_CONNECT);
        byte_array_add_byte(byte_array, 1);
        byte_array_add_byte(byte_array, 1);
        byte_array_add_byte(byte_array, 0);
    } else {
        // Otherwise, push the root node and the interface

        // Load the interface
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, 0, vm->word_size);
        byte_array_add_byte(byte_array, 0);

        // Load the tree
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, tree_addr, vm->word_size);
        byte_array_add_byte(byte_array, 1);

        // Push the tree and the interface
        byte_array_add_byte(byte_array, OP_PUSH);
        byte_array_add_byte(byte_array, 0);
        byte_array_add_byte(byte_array, 1);
    }
    byte_array_add_byte(byte_array, OP_RETURN);

    // Set the code for the VM
    tape_set(vm->tape, byte_array_get_bytes(byte_array));

    printf("Print instructions\n");
    vm_print(vm);
    printf("Finish\n");

    // Execute the instructions
    _vm_exec(vm);

    // DEBUG PRINT
    // struct Tree* debug = vm_readback(vm);
    // tree_print(debug);

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
    if (eq_stack_size(vm->active_pairs) == 0) {
        return EvalFinished;
    }
    struct Equation eq = eq_stack_pop(vm->active_pairs);
    printf("OUT eq: %llx %llx\n", (size_t)eq.agent0, (size_t)eq.agent1);

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
                printf("Agent 0 (%llx) is a name, redirecting it to %llx\n",
                    (size_t)eq.agent0, (size_t)eq.agent1);
            } else {
                // x is an indirection
                struct Equation new_eq;
                new_eq.agent0 = agent_get_port(eq.agent0, 0);
                new_eq.agent1 = eq.agent1;
                eq_stack_push(vm->active_pairs, new_eq);
                agent_free(eq.agent0);
                printf("Agent 0 (%llx) is an indirection to %llx. Pushing new equation with %llx.\n",
                    (size_t)eq.agent0, (size_t)new_eq.agent0, (size_t)eq.agent1);
            }
        }
    } else {
        if (agent_get_port(eq.agent1, 0) == NULL) {
            // y is a name
            agent_set_port(eq.agent1, 0, eq.agent0);
            printf("Agent 1 (%llx) is a name, redirecting it to %llx\n",
                    (size_t)eq.agent1, (size_t)eq.agent0);
        } else {
            // y is an indirection
            // Alpha(x1, ..., xn) = y and x = y
            struct Equation new_eq;
            new_eq.agent0 = eq.agent0;
            new_eq.agent1 = agent_get_port(eq.agent1, 0);
            eq_stack_push(vm->active_pairs, new_eq);
            agent_free(eq.agent1);
            printf("Agent 1 (%llx) is an indirection to %llx. Pushing new equation with %llx.\n",
                    (size_t)eq.agent1, (size_t)new_eq.agent1, (size_t)eq.agent0);
        }
    }

    if (continue_eval == FALSE) {
        return EvalRunning;
    }

    assert(agent0_type < agent1_type);

    // Set up registers
    // if (agent0_type > agent1_type) {
    //     printf("WARNING: agent0_type (%d) > agent1_type (%d)\n", agent0_type,
    //         agent1_type);
    //     struct Agent* temp = eq.agent0;
    //     eq.agent0 = eq.agent1;
    //     eq.agent1 = temp;
    // }
    for (uint8_t i = 0; i < MAX_AUX_PORT_NUM; i++) {
        vm->reg[i] = agent_get_port(eq.agent0, i);
        vm->reg[MAX_AUX_PORT_NUM + i] = agent_get_port(eq.agent1, i);
    }
    // Defensive programming: null out all the other registers
    for (uint8_t i = 2 * MAX_AUX_PORT_NUM; i < MAX_REG_SIZE; i++) {
        vm->reg[i] = NULL;
    }

    // Load the instructions for this pair
    // if (agent0_type > agent1_type) {
    //     printf("WARNING: agent0_type (%d) > agent1_type (%d)\n", agent0_type,
    //         agent1_type);
    //     uint8_t temp = agent0_type;
    //     agent0_type = agent1_type;
    //     agent1_type = temp;
    // }
    uint8_t code_index = (agent0_type - ID_K) * 4 + (agent1_type - ID_E);
    tape_set(vm->tape, CodeTable[code_index]);
    if (CodeNameTable[code_index] == NULL) {
        printf("ERROR: no rule found for %d - %d\n", agent0_type, agent1_type);
    } else {
        printf("Executing reduction rule %s\n", CodeNameTable[code_index]);
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
            printf("\tCreated agent at %llx with type %s\n",
                (size_t)vm->reg[index], AgentNameTable[type]);
            break;
        }
        case OP_MKNAME: {
            uint8_t index = _vm_read_byte(vm);
            vm->reg[index] = agent_make_name();
            printf("\tCreated name at %llx\n",
                (size_t)vm->reg[index]);
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
            printf("\tIN eq: %llx %llx\n", (size_t)eq.agent0, (size_t)eq.agent1);
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

struct Tree* _vm_readback_from_agent(struct VM* vm, struct Agent* agent) {
    // Reconstruct the tree from the root
    switch (agent_get_type(agent)) {
        case ID_NAME: {
            return tree_make_program(program_with_label("NAME", program_make_leaf()));
            // TODO when reading back a name, F could be DUP and K could be ERA
        }
        case ID_K: {
            return tree_make_program(program_make_leaf());
        }
        case ID_S: {
            struct Tree* tree =
                _vm_readback_from_agent(vm, agent_get_port(agent, 0));
            return tree_make_program(program_make_stem(tree_get_program(tree)));
        }
        case ID_F: {
            struct Tree* tree0 =
                _vm_readback_from_agent(vm, agent_get_port(agent, 0));
            struct Tree* tree1 =
                _vm_readback_from_agent(vm, agent_get_port(agent, 1));
            return tree_make_program(program_make_fork(
                tree_get_program(tree0), tree_get_program(tree1)));
        }
        case ID_A: {
            struct Tree* tree0 =
                _vm_readback_from_agent(vm, _vm_get_pair(vm, agent));
            struct Tree* tree1 =
                _vm_readback_from_agent(vm, agent_get_port(agent, 0));
            return tree_make_apply(tree0, tree1);
        }
        case ID_T: {
            struct Tree* tree0 =
                _vm_readback_from_agent(vm, _vm_get_pair(vm, agent));
            struct Tree* tree1 =
                _vm_readback_from_agent(vm, agent_get_port(agent, 0));
            struct Tree* tree2 =
                _vm_readback_from_agent(vm, agent_get_port(agent, 1));
            struct Tree* left =
                tree_make_program(
                    program_make_fork(
                        tree_get_program(tree0), tree_get_program(tree1)));
            return tree_make_apply(left, tree2);
        }
    }
    return NULL;
}

// TODO the interface might not be on the eq_stack, if it is connected to an
// agent
struct Tree* vm_readback(struct VM* vm) {
    // Get the interface
    struct Agent* interface = agent_heap_get(vm->heap, 0);
    // Get the interface's pair in the stack
    struct Agent* root = _vm_get_pair(vm, interface);
    if (root == NULL) {
        root = agent_get_port(interface, 0);
    }

    return _vm_readback_from_agent(vm, root);
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
