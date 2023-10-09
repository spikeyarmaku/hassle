#include "tree.h"

// Syntax
// M, N ::= Δ | M N

// TODO An alternative representation could save reallocation in `tree_apply`.
// The rust implementation in the original tree calculus github repo defines a
// "program" (or "value") type, which can be either a leaf, stem or fork, and
// contains Tree references. It then defines a "tree" type, which either holds a
// value (of type "program"), or is an application of two "tree"s.

struct Tree {
    uint8_t aux_data;
    // llcccccc - l = leaf type, c = child count
    // leaf types: 00 = delta, 01 = symbol, 10 = str, 11 = rat
    union {
        struct Tree** children;
        char* str_val;
        Rational_t* rat_val;
    };
};

void _tree_set_child_count(struct Tree* tree, uint8_t child_count) {
    if (child_count > 63) {
        printf("_tree_set_child_count: child count must be less than 64\n");
        exit(1);
    }

    tree->aux_data = child_count;
}

void _tree_set_leaf_type(struct Tree* tree, uint8_t leaf_type) {
    tree->aux_data = leaf_type << 6;
}

size_t _tree_size(struct Tree* tree) {
    size_t size = 1;
    for (uint8_t i = 0; i < tree_child_count(tree); i++) {
        size += _tree_size(tree->children[i]);
    }
    return size;
}

struct Tree* tree_make(uint8_t child_count) {
    struct Tree* tree = allocate_mem("tree_make", NULL,
        sizeof(struct Tree));
    _tree_set_child_count(tree, child_count);
    if (child_count > 0) {
        tree->children = allocate_mem("tree_make/children", NULL,
            sizeof(struct Tree) * child_count);
    } else {
        tree->children = NULL;
    }
    return tree;
}

struct Tree* tree_make_sym(char* str) {
    struct Tree* tree = tree_make(0);
    _tree_set_leaf_type(tree, LEAF_TYPE_SYMBOL);
    tree->str_val = str;
    return tree;
}

struct Tree* tree_make_str(char* str) {
    struct Tree* tree = tree_make(0);
    _tree_set_leaf_type(tree, LEAF_TYPE_STRING);
    tree->str_val = str;
    return tree;
}

struct Tree* tree_make_rat(Rational_t* rat) {
    struct Tree* tree = tree_make(0);
    _tree_set_leaf_type(tree, LEAF_TYPE_RATIONAL);
    tree->rat_val = rat;
    return tree;
}

void tree_set_child(struct Tree* tree, uint8_t child_index, struct Tree* child)
{
    tree->children[child_index] = child;
}

struct Tree* tree_copy(struct Tree* tree) {
    uint8_t child_count = tree_child_count(tree);
    struct Tree* new_tree;
    if (tree_is_value(tree)) {
        if (tree_leaf_type(tree) == LEAF_TYPE_RATIONAL) {
            new_tree = tree_make_rat(rational_copy(tree->rat_val));
        } else {
            char* str_val = allocate_mem("tree_copy/str", NULL,
                sizeof(char) * (strlen(tree->str_val) + 1));
            strcpy(str_val, tree->str_val);
            new_tree = tree_make_str(str_val);
        }
        _tree_set_leaf_type(new_tree, tree_leaf_type(tree));
    } else {
        new_tree = tree_make(child_count);
        for (uint8_t i = 0; i < child_count; i++) {
            new_tree->children[i] = tree_copy(tree->children[i]);
        }
    }
    return new_tree;
}

void tree_free_toplevel(struct Tree* tree) {
    free_mem("tree_free/children", tree->children);
    free_mem("tree_free", tree);
}

uint8_t tree_child_count(struct Tree* tree) {
    // Drop the first two bits of the aux data
    return tree->aux_data & ~(3 << 6);
}

void tree_free(struct Tree* tree) {
    if (tree_is_value(tree) == TRUE) {
        switch (tree_leaf_type(tree)) {
            case LEAF_TYPE_RATIONAL: {
                free_mem("tree_free/rat", tree->rat_val);
                break;
            }
            case LEAF_TYPE_STRING: {
                free_mem("tree_free/str", tree->str_val);
                break;
            }
            case LEAF_TYPE_SYMBOL: {
                free_mem("tree_free/str", tree->str_val);
                break;
            }
            default: {
                break;
            }
        }
    } else {
        for (uint8_t i = 0; i < tree_child_count(tree); i++) {
            tree_free(tree->children[i]);
        }
    }
    tree_free_toplevel(tree);
}

struct Tree* tree_get_child(struct Tree* tree, uint8_t child_index) {
    return tree->children[child_index];
}

struct Tree* tree_apply(struct Tree* tree1, struct Tree* tree2) {
    if (tree_is_value(tree1)) {
        printf("tree_apply: trying to apply a tree to a value\n"); exit(1);
    }
    uint8_t child_count = tree_child_count(tree1);
    tree1->children = allocate_mem("tree_apply", tree1->children,
        sizeof(struct Tree) * (child_count + 1));
    tree1->children[child_count] = tree2;
    _tree_set_child_count(tree1, child_count + 1);
    return tree1;
}

void tree_serialize(Serializer_t* serializer, struct Tree* tree) {
    serializer_write(serializer, tree->aux_data);
    if (tree_is_value(tree)) {
        if (tree_leaf_type(tree) == LEAF_TYPE_RATIONAL) {
            rational_serialize(serializer, tree->rat_val);
        } else {
            serializer_write_string(serializer, tree->str_val);
        }
    } else {
        for (uint8_t i = 0; i < tree_child_count(tree); i++) {
            tree_serialize(serializer, tree->children[i]);
        }
    }
}

struct Tree* _tree_deserialize_subtree(uint8_t** data) {
    struct Tree* tree = tree_make(**data);
    *data += sizeof(uint8_t);
    for (uint8_t i = 0; i < tree_child_count(tree); i++) {
        tree->children[i] = _tree_deserialize_subtree(data);
    }
    return tree;
}

struct Tree* tree_deserialize(Serializer_t* serializer) {
    uint8_t aux_data = serializer_read(serializer);
    switch (aux_data) {
        case LEAF_TYPE_DELTA: {
            // Either a leaf without value, or a tree
            uint8_t child_count = aux_data;
            struct Tree* tree = tree_make(child_count);
            for (uint8_t i = 0; i < child_count; i++) {
                tree->children[i] = tree_deserialize(serializer);
            }
            return tree;
        }
        case LEAF_TYPE_RATIONAL: {
            return tree_make_rat(rational_deserialize(serializer));
        }
        case LEAF_TYPE_STRING: {
            return tree_make_str(serializer_read_string(serializer));
        }
        case LEAF_TYPE_SYMBOL: {
            return tree_make_sym(serializer_read_string(serializer));
        }
        default: {
            printf("tree_deserialize: invalid aux_data\n"); exit(1);
        }
    }
}

BOOL tree_is_value(struct Tree* tree) {
    return tree_leaf_type(tree) == 0 ? FALSE : TRUE;
}

BOOL tree_is_symbol(char* symbol, struct Tree* tree) {
    if (tree_is_value(tree) &&
        tree_leaf_type(tree) == LEAF_TYPE_SYMBOL &&
        strcmp(symbol, tree_get_sym(tree)) == 0)
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t tree_leaf_type(struct Tree* tree) {
    return tree->aux_data >> 6;
}

char* tree_get_sym(struct Tree* tree) {
    return tree->str_val;
}

char* tree_get_str(struct Tree* tree) {
    return tree->str_val;
}

Rational_t* tree_get_rat(struct Tree* tree) {
    return tree->rat_val;
}

void tree_print(struct Tree* tree) {
    if (tree_is_value(tree)) {
        printf("[");
        switch (tree_leaf_type(tree)) {
            case LEAF_TYPE_RATIONAL: {
                rational_print(tree->rat_val);
                break;
            }
            case LEAF_TYPE_STRING: {
                printf("\"%s\"", tree->str_val);
                break;
            }
            case LEAF_TYPE_SYMBOL: {
                printf("%s", tree->str_val);
                break;
            }
        }
        printf("]");
    } else {
        for (uint8_t i = 0; i < tree_child_count(tree); i++) {
            printf(" ");
            tree_print(tree->children[i]);
        }
    }
}
