#include "tree.h"

struct Tree {
    uint8_t type;
    union {
        struct Program* program;
        struct Tree* apply[2];
    };
};

struct Tree* tree_make_program(struct Program* program) {
    struct Tree* tree = allocate_mem("tree_make_program", NULL,
        sizeof(struct Tree));
    tree->type = TREE_TYPE_PROGRAM;
    tree->program = program;
    return tree;
}

struct Tree* tree_make_apply(struct Tree* tree0, struct Tree* tree1) {
    struct Tree* tree = allocate_mem("tree_make_program", NULL,
        sizeof(struct Tree));
    tree->type = TREE_TYPE_APPLY;
    tree->apply[0] = tree0;
    tree->apply[1] = tree1;
    return tree;
}

struct Tree* tree_apply(struct Tree* tree0, struct Tree* tree1) {
    if (tree_get_type(tree0) != TREE_TYPE_PROGRAM ||
        tree_get_type(tree1) != TREE_TYPE_PROGRAM)
    {
        // If one of the children is not program, make an explicit application
        return tree_make_apply(tree0, tree1);
    } else {
        if (program_get_type(tree_get_program(tree0)) == PROGRAM_TYPE_VALUE) {
            // If the first program is a value, make an explicit application
            return tree_make_apply(tree0, tree1);
        } else {
            // Otherwise, try to apply one program to the other
            struct Tree* result =
                tree_apply_programs(
                    program_copy(tree_get_program(tree0)),
                    program_copy(tree_get_program(tree1)));
            tree_free(tree0);
            tree_free(tree1);
            return result;
        }
    }
}

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)
struct Tree* tree_apply_programs(struct Program* prg0, struct Program* prg1) {
    if (program_apply(prg0, prg1) == TRUE) {
        return tree_make_program(prg0);
    } else {
        switch (program_get_type(program_get_child(prg0, 0))) {
            case PROGRAM_TYPE_LEAF: {
                // K rule - ΔΔyz = y
                struct Tree* result =
                    tree_make_program(program_copy(program_get_child(prg0, 1)));
                program_free(prg0);
                program_free(prg1);
                return result;

            }
            case PROGRAM_TYPE_STEM: {
                // S rule - Δ(Δx)yz = yz(xz)
                struct Program* x =
                    program_copy(
                        program_get_child(program_get_child(prg0, 0), 0));
                struct Program* y =
                    program_copy(program_get_child(prg0, 1));
                struct Program* z0 = prg1;
                struct Program* z1 = program_copy(z0);
                program_free(prg0);
                return
                    tree_make_apply(
                        tree_make_apply(
                            tree_make_program(y), tree_make_program(z0)),
                        tree_make_apply(
                            tree_make_program(x), tree_make_program(z1)));
            }
            case PROGRAM_TYPE_FORK: {
                // F rule - Δ(Δwx)yz = zwx
                struct Program* x =
                    program_copy(
                        program_get_child(program_get_child(prg0, 0), 1));
                struct Program* z = prg1;
                struct Program* w =
                    program_copy(
                        program_get_child(program_get_child(prg0, 0), 0));
                program_free(prg0);
                return
                    tree_make_apply(tree_make_apply(
                        tree_make_program(z), tree_make_program(w)),
                        tree_make_program(x));
            }
            case PROGRAM_TYPE_VALUE: {
                fatal("tree_apply_programs: Trying to apply to a program\n");
                return NULL;
            }
            default: {
                fatal("tree_apply_programs: Invalid program type: %d\n",
                    program_get_type(prg0));
                return NULL;
            }
        }
    }
}

uint8_t tree_get_type(struct Tree* tree) {
    return tree->type;
}

struct Program* tree_get_program(struct Tree* tree) {
    return tree->program;
}

struct Tree* tree_get_apply(struct Tree* tree, uint8_t index) {
    return tree->apply[index];
}

struct Tree* tree_copy(struct Tree* tree) {
    switch (tree->type) {
        case TREE_TYPE_PROGRAM: {
            return tree_make_program(program_copy(tree->program));
        }
        case TREE_TYPE_APPLY: {
            return tree_make_apply(
                tree_copy(tree->apply[0]), tree_copy(tree->apply[1]));
        }
        default: {
            fatal("tree_copy: invalid tree type %d\n", tree->type);
            return NULL;
        }
    }
}

void tree_free(struct Tree* tree) {
    if (tree == NULL) {
        return;
    }
    switch (tree->type) {
        case TREE_TYPE_PROGRAM: {
            program_free(tree->program);
            break;
        }
        case TREE_TYPE_APPLY: {
            tree_free(tree->apply[0]);
            tree_free(tree->apply[1]);
            break;
        }
        default: {
            fatal("tree_free: invalid type %d\n", tree->type);
        }
    }
    free_mem("tree_free", tree);
}

void tree_serialize(Serializer_t* serializer, struct Tree* tree) {
    serializer_write(serializer, tree->type);
    switch (tree->type) {
        case TREE_TYPE_PROGRAM: {
            program_serialize(serializer, tree->program);
            break;
        }
        case TREE_TYPE_APPLY: {
            tree_serialize(serializer, tree->apply[0]);
            tree_serialize(serializer, tree->apply[1]);
            break;
        }
        default: {
            fatal("tree_serialize: invalid tree type %d\n", tree->type);
        }
    }
}

struct Tree* tree_deserialize(Serializer_t* serializer) {
    // TODO
    return NULL;
}

void tree_print(struct Tree* tree) {
    switch (tree_get_type(tree)) {
        case TREE_TYPE_PROGRAM: {
            program_print(tree_get_program(tree));
            break;
        }
        case TREE_TYPE_APPLY: {
            printf("(");
            tree_print(tree_get_apply(tree, 0));
            printf(" ");
            tree_print(tree_get_apply(tree, 1));
            printf(")");
            break;
        }
    }
}

size_t tree_get_size(struct Tree* tree) {
    switch (tree_get_type(tree)) {
        case TREE_TYPE_PROGRAM: {
            return program_get_size(tree_get_program(tree));
        }
        case TREE_TYPE_APPLY: {
            return tree_get_size(tree_get_apply(tree, 0)) +
                tree_get_size(tree_get_apply(tree, 1));
        }
        default: {
            fatal("tree_get_size: invalid tree type %d\n", tree_get_type(tree));
            return 0;
        }
    }
}
