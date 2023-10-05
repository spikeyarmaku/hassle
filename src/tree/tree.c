#include "tree.h"

// Syntax
// M, N ::= Δ | M N

struct Tree {
    uint8_t child_count;
    struct Tree** children;
};

size_t _tree_size(struct Tree* tree) {
    size_t size = 0;
    for (uint8_t i = 0; i < tree->child_count; i++) {
        size += _tree_size(tree->children[i]);
    }
    return size + 1;
}

struct Tree* tree_make(uint8_t child_count) {
    struct Tree* tree = allocate_mem("tree_make", NULL,
        sizeof(struct Tree));
    tree->child_count = child_count;
    if (child_count > 0) {
        tree->children = allocate_mem("tree_make/children", NULL,
            sizeof(struct Tree) * child_count);
    } else {
        tree->children = NULL;
    }
    return tree;
}

struct Tree* tree_copy(struct Tree* tree) {
    struct Tree* new_tree = tree_make(tree->child_count);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        new_tree->children[i] = tree_copy(tree->children[i]);
    }
    return new_tree;
}

void tree_free_toplevel(struct Tree* tree) {
    free_mem("tree_free/children", tree->children);
    free_mem("tree_free", tree);
}

void tree_free(struct Tree* tree) {
    for (uint8_t i = 0; i < tree->child_count; i++) {
        tree_free(tree->children[i]);
    }
    tree_free_toplevel(tree);
}

uint8_t tree_child_count(struct Tree* tree) {
    return tree->child_count;
}

struct Tree* tree_get_child(struct Tree* tree, uint8_t child_index) {
    return tree->children[child_index];
}

struct Tree* tree_apply(struct Tree* tree1, struct Tree* tree2) {
    tree1->children = allocate_mem("tree_apply", tree1->children,
        sizeof(struct Tree) * (tree1->child_count + 1));
    tree1->children[tree1->child_count] = tree2;
    tree1->child_count++;
    return tree1;
}

// Write its child count and all of its children's child_count into the array,
// and return the next address that can be written to
void _tree_serialize_subtree(struct Tree* tree, uint8_t** data) {
    **data = tree->child_count;
    *data += sizeof(uint8_t);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        _tree_serialize_subtree(tree->children[i], data);
    }
}

uint8_t* tree_serialize(struct Tree* tree, size_t* out_size) {
    // printf("tree size: %llu\n", _tree_size(tree));
    size_t array_size = _tree_size(tree);
    uint8_t* data = allocate_mem("tree_serialize", NULL,
        sizeof(uint8_t) * array_size);
    uint8_t* data_mut = data;
    _tree_serialize_subtree(tree, &data_mut);
    
    *out_size = array_size;
    return data;
}

struct Tree* _tree_deserialize_subtree(uint8_t** data) {
    struct Tree* tree = tree_make(**data);
    *data += sizeof(uint8_t);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        tree->children[i] = _tree_deserialize_subtree(data);
    }
    return tree;
}

struct Tree* tree_deserialize(uint8_t* data) {
    return _tree_deserialize_subtree(&data);
}

void tree_print(struct Tree* tree) {
    printf("%d", tree->child_count);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        printf(" ");
        tree_print(tree->children[i]);
    }
}
