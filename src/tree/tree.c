#include "tree.h"

struct Tree {
    uint8_t type;
    union {
        struct Program* value;
        struct Tree* apply[2];
    };
};

struct Tree* tree_make_value(struct Program* value) {
    struct Tree* tree = allocate_mem("tree_make_value", NULL,
        sizeof(struct Tree));
    tree->type = TREE_TYPE_VALUE;
    tree->value = value;
    return tree;
}

struct Tree* tree_make_apply(struct Tree* tree0, struct Tree* tree1) {
    struct Tree* tree = allocate_mem("tree_make_value", NULL,
        sizeof(struct Tree));
    tree->type = TREE_TYPE_APPLY;
    tree->apply[0] = tree0;
    tree->apply[1] = tree1;
    return tree;
}

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)
struct Tree* tree_apply_values(struct Program* prg0, struct Program* prg1) {
    if (program_apply(prg0, prg1) == TRUE) {
        return tree_make_value(prg0);
    } else {
        switch (program_get_type(program_get_child(prg0, 0))) {
            case PROGRAM_TYPE_LEAF: {
                // K rule - ΔΔyz = y
                struct Tree* result =
                    tree_make_value(program_copy(program_get_child(prg0, 1)));
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
                            tree_make_value(y), tree_make_value(z0)),
                        tree_make_apply(
                            tree_make_value(x), tree_make_value(z1)));
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
                        tree_make_value(z), tree_make_value(w)),
                        tree_make_value(x));
            }
            default: {
                fatal("tree_apply_values: Invalid program type: %d\n",
                    program_get_type(prg0));
                return NULL;
            }
        }
    }
}

uint8_t tree_get_type(struct Tree* tree) {
    return tree->type;
}

struct Program* tree_get_value(struct Tree* tree) {
    return tree->value;
}

struct Tree* tree_get_apply(struct Tree* tree, uint8_t index) {
    return tree->apply[index];
}

struct Tree* tree_copy(struct Tree* tree) {
    switch (tree->type) {
        case TREE_TYPE_VALUE: {
            return tree_make_value(program_copy(tree->value));
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
        case TREE_TYPE_VALUE: {
            program_free(tree->value);
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
        case TREE_TYPE_VALUE: {
            program_serialize(serializer, tree->value);
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
        case TREE_TYPE_VALUE: {
            program_print(tree_get_value(tree));
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
