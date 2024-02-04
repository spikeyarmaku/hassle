#include "convert.h"

// TODO use names instead of copy
// calculate left child, and connect it to the F node, and use a name instead of
// the right child
size_t _code_from_program(struct ByteArray* byte_array, struct Program* prg,
    size_t next_id, size_t word_size)
{
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
                _code_from_program(byte_array, program_get_child(prg, 0),
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
                _code_from_program(byte_array, program_get_child(prg, 0),
                    next_id, word_size);
            size_t child1_addr =
                _code_from_program(byte_array, program_get_child(prg, 1),
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

size_t _code_from_tree(struct ByteArray* byte_array, struct Tree* tree,
    size_t next_id, size_t word_size)
{
    switch (tree_get_type(tree)) {
        case TREE_TYPE_PROGRAM: {
            struct Program* prg = tree_get_program(tree);
            return _code_from_program(byte_array, prg, next_id, word_size);
        }
        case TREE_TYPE_APPLY: {
            struct Tree* subtree0 = tree_get_apply(tree, 0);
            struct Tree* subtree1 = tree_get_apply(tree, 1);
            // Create instructions for the second subtree
            size_t child0_addr = _code_from_tree(byte_array, subtree0,
                next_id, word_size);
            size_t child1_addr = _code_from_tree(byte_array, subtree1,
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
                // If its right child is a program, connect its first aux port
                // to reg1
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
            if (tree_get_type(subtree0) == TREE_TYPE_PROGRAM) {
                // If the left child is a program, push the result and the App
                // agent to the stack
                byte_array_add_byte(byte_array, OP_PUSH);
                byte_array_add_byte(byte_array, 1);
                byte_array_add_byte(byte_array, 0);
            } else {
                // Otherwise, connect it to the left child's last aux port
                byte_array_add_byte(byte_array, OP_CONNECT);
                byte_array_add_byte(byte_array, 1);
                byte_array_add_byte(byte_array, 1);
                byte_array_add_byte(byte_array, 0);
            }
            // Store it
            byte_array_add_byte(byte_array, OP_STORE);
            byte_array_add_byte(byte_array, 0);
            byte_array_add_word(byte_array, child1_addr + 1, word_size);
            return child1_addr + 1;
        }
    }
    return 0;
}

struct ByteArray* convert_from_tree(struct Tree* tree, uint8_t word_size) {
    struct ByteArray* byte_array = byte_array_make();

    // Create the interface
    byte_array_add_byte(byte_array, OP_MKNAME);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_byte(byte_array, OP_STORE);
    byte_array_add_byte(byte_array, 0);
    byte_array_add_word(byte_array, 0, word_size);
    // Convert the tree to a list of instructions
    size_t tree_addr = _code_from_tree(byte_array, tree, 1, word_size);
    if (tree_get_type(tree) == TREE_TYPE_APPLY) {
        // If the tree is an application, connect its p1 to the interface

        // Load the interface
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, 0, word_size);
        byte_array_add_byte(byte_array, 0);

        // Load the tree
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, tree_addr, word_size);
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
        byte_array_add_word(byte_array, 0, word_size);
        byte_array_add_byte(byte_array, 0);

        // Load the tree
        byte_array_add_byte(byte_array, OP_LOAD);
        byte_array_add_word(byte_array, tree_addr, word_size);
        byte_array_add_byte(byte_array, 1);

        // Push the tree and the interface
        byte_array_add_byte(byte_array, OP_PUSH);
        byte_array_add_byte(byte_array, 0);
        byte_array_add_byte(byte_array, 1);
    }
    byte_array_add_byte(byte_array, OP_RETURN);

    return byte_array;
}
