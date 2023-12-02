#include "kin.h"

struct Kin {
    BOOL parent;
    struct Tree* tree;
};

struct Kin* kin_make(BOOL is_parent, struct Tree* tree) {
    struct Kin* kin = allocate_mem("_kin_make", NULL, sizeof(struct Kin));
    kin->parent = is_parent;
    kin->tree = tree;
    return kin;
}

void kin_free(struct Kin* kin) {
    tree_free(kin->tree);
    free_mem("_kin_free", kin);
}


BOOL kin_is_parent(struct Kin* kin) {
    return kin->parent;
}

struct Tree* kin_get_tree(struct Kin* kin) {
    return kin->tree;
}

void kin_serialize(Serializer_t* serializer, struct Kin* kin) {
    serializer_write(serializer, kin->parent == TRUE ? 1 : 0);
    tree_serialize(serializer, kin->tree);
}

struct Kin* kin_deserialize(Serializer_t* serializer) {
    BOOL is_parent = serializer_read(serializer) == 1 ? TRUE : FALSE;
    struct Tree* tree = tree_deserialize(serializer);
    return kin_make(is_parent, tree);
}

struct Tree* kin_extract_tree(struct Kin* kin) {
    struct Tree* result = kin->tree;
    free_mem("kin_extract_tree", kin);
    return result;
}
