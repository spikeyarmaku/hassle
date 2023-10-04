#include "tree.h"

// Syntax
// M, N ::= Δ | M N

struct Tree {
    uint8_t child_count;
    struct Tree** children;
};

size_t _tree_size(struct Tree* tree) {
    size_t size = tree->child_count;
    for (uint8_t i = 0; i < tree->child_count; i++) {
        size += _tree_size(tree->children[i]);
    }
    return size;
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

size_t tree_child_count(struct Tree* tree) {
    return tree->child_count;
}

struct Tree* tree_get_child(struct Tree* tree, size_t child_index) {
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
uint8_t* _tree_serialize_subtree(struct Tree* tree, uint8_t* data) {
    *data = tree->child_count;
    uint8_t* new_data = data + sizeof(uint8_t);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        uint8_t* new_data =
            _tree_serialize_subtree(tree->children[i], new_data);
    }
    return new_data;
}

uint8_t* tree_serialize(struct Tree* tree, size_t* out_size) {
    size_t array_size = _tree_size(tree);
    uint8_t* data = allocate_mem("tree_serialize", NULL,
        sizeof(uint8_t) * array_size);
    _tree_serialize_subtree(tree, data);
    
    *out_size = array_size;
    return data;
}

struct Tree* _tree_deserialize_subtree(uint8_t** data) {
    struct Tree* tree = tree_make(**data);
    *data += sizeof(uint8_t);
    for (uint8_t i = 0; i < tree->child_count; i++) {
        tree->children[i] = _tree_deserialize_subtree(&data);
    }
}

struct Tree* tree_deserialize(uint8_t* data) {
    return _tree_deserialize_subtree(&data);
}
