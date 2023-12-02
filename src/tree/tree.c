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
        // Otherwise, try to apply one program to the other
        // struct Tree* result =
        //     tree_apply_programs(
        //         program_copy(tree_get_program(tree0)),
        //         program_copy(tree_get_program(tree1)));
        // tree_free(tree0);
        // tree_free(tree1);
        // return result;
        struct Program* prg0 = tree_extract_program(tree0);
        struct Program* prg1 = tree_extract_program(tree1);
        return tree_apply_programs(prg0, prg1);
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
        if (program_get_label(program_get_child(prg0, 0)) != NULL) {
            // struct Tree* result =
            //     tree_make_program(program_copy(program_get_child(prg0, 1)));
            // program_free(prg0);
            // program_free(prg1);
            // return result;
            struct ProgramPair subprograms = program_extract_subprograms(prg0);
            struct Tree* result = tree_make_program(subprograms.prg1);
            program_free(subprograms.prg0);
            program_free(prg1);
            return result;
        } else {
            switch (program_get_type(program_get_child(prg0, 0))) {
                case PROGRAM_TYPE_LEAF: {
                    // K rule - ΔΔyz = y
                    // struct Tree* result =
                    //     tree_make_program(
                    //         program_copy(program_get_child(prg0, 1)));
                    // program_free(prg0);
                    // program_free(prg1);
                    // return result;
                    printf("K rule\n");
                    struct ProgramPair subprograms =
                        program_extract_subprograms(prg0);
                    struct Tree* result = tree_make_program(subprograms.prg1);
                    program_free(subprograms.prg0);
                    program_free(prg1);
                    return result;
                }
                case PROGRAM_TYPE_STEM: {
                    // S rule - Δ(Δx)yz = yz(xz)
                    // struct Program* x =
                    //     program_copy(
                    //         program_get_child(program_get_child(prg0, 0), 0));
                    // struct Program* y =
                    //     program_copy(program_get_child(prg0, 1));
                    // struct Program* z0 = prg1;
                    // struct Program* z1 = program_copy(z0);
                    // program_free(prg0);
                    // return
                    //     tree_make_apply(
                    //         tree_make_apply(
                    //             tree_make_program(y), tree_make_program(z0)),
                    //         tree_make_apply(
                    //             tree_make_program(x), tree_make_program(z1)));
                    printf("S rule\n");
                    struct ProgramPair subprograms =
                        program_extract_subprograms(prg0);
                    struct ProgramPair sub_subprograms =
                        program_extract_subprograms(subprograms.prg0);
                    struct Program* x = sub_subprograms.prg0;
                    struct Program* y = subprograms.prg1;
                    struct Program* z0 = prg1;
                    printf("Copying...");
                    struct Program* z1 = program_copy(prg1);
                    printf("finished\n");
                    return
                        tree_make_apply(
                            tree_apply_programs(y, z0),
                            tree_apply_programs(x, z1));
                        // tree_make_apply(
                        //     tree_make_apply(
                        //         tree_make_program(y), tree_make_program(z0)),
                        //     tree_make_apply(
                        //         tree_make_program(x), tree_make_program(z1)));
                }
                case PROGRAM_TYPE_FORK: {
                    // F rule - Δ(Δwx)yz = zwx
                    // struct Program* x =
                    //     program_copy(
                    //         program_get_child(program_get_child(prg0, 0), 1));
                    // struct Program* z = prg1;
                    // struct Program* w =
                    //     program_copy(
                    //         program_get_child(program_get_child(prg0, 0), 0));
                    // program_free(prg0);
                    // return
                    //     tree_make_apply(tree_make_apply(
                    //         tree_make_program(z), tree_make_program(w)),
                    //         tree_make_program(x));
                    printf("F rule\n");
                    struct ProgramPair subprograms =
                        program_extract_subprograms(prg0);
                    struct ProgramPair sub_subprograms =
                        program_extract_subprograms(subprograms.prg0);
                    struct Program* w = sub_subprograms.prg0;
                    struct Program* x = sub_subprograms.prg1;
                    program_free(subprograms.prg1);
                    struct Program* z = prg1;
                    return
                        tree_make_apply(tree_apply_programs(z, w),
                            tree_make_program(x));
                        // tree_make_apply(tree_make_apply(
                        //     tree_make_program(z), tree_make_program(w)),
                        //     tree_make_program(x));
                }
                default: {
                    fatal("tree_apply_programs: Invalid program type: %d\n",
                        program_get_type(prg0));
                    return NULL;
                }
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

struct TreePair tree_extract_subtrees(struct Tree* tree) {
    assert(tree->type == TREE_TYPE_APPLY);
    struct TreePair result;
    result.tree0 = tree->apply[0];
    result.tree1 = tree->apply[1];
    free_mem("tree_extract_subtrees", tree);
    return result;
}

struct Program* tree_extract_program(struct Tree* tree) {
    assert(tree->type == TREE_TYPE_PROGRAM);
    struct Program* result = tree->program;
    free_mem("tree_extract_program", tree);
    return result;
}

BOOL tree_is_reference(struct Tree* tree) {
    if (tree->type == TREE_TYPE_PROGRAM) {
        if (program_get_type(tree->program) == PROGRAM_TYPE_LEAF) {
            return program_get_label(tree->program) != NULL ? TRUE : FALSE;
        }
    }
    return FALSE;
}

char* tree_get_label(struct Tree* tree) {
    if (tree->type == TREE_TYPE_PROGRAM) {
        return program_get_label(tree_get_program(tree));
    }
    return NULL;
}

// If the tree is an application, leave it, otherwise make it an application if
// possible
struct Tree* tree_extract(struct Tree* tree) {
    if (tree->type == TREE_TYPE_APPLY) {
        return tree;
    } else {
        if (tree_is_reference(tree) == TRUE) {
            return tree;
        } else {
            switch (program_get_type(tree->program)) {
                case PROGRAM_TYPE_LEAF: {
                    return tree;
                }
                case PROGRAM_TYPE_STEM: {
                    struct Program* child0 = program_make_leaf();
                    struct Program* child1 =
                        program_copy(program_get_child(tree->program, 0));
                    tree_free(tree);
                    return
                        tree_make_apply(
                            tree_make_program(child0),
                            tree_make_program(child1));
                }
                case PROGRAM_TYPE_FORK: {
                    struct Program* child0 =
                        program_make_stem(
                            program_copy(program_get_child(tree->program, 0)));
                    struct Program* child1 =
                        program_copy(program_get_child(tree->program, 1));
                    tree_free(tree);
                    return
                        tree_make_apply(
                            tree_make_program(child0),
                            tree_make_program(child1));
                }
                default: {
                    fatal("tree_get_child: invalid program type %d\n",
                        program_get_type(tree->program));
                    return NULL;
                }
            }
        }
    }
}
