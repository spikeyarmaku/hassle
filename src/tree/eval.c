#include "eval.h"

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)

struct Tree* eval(struct Tree* tree) {
    if (tree_child_count(tree) > 2) {
        switch (tree_child_count(tree_get_child(tree, 0))) {
            case 0: {
                // (K) rule
                struct Tree* result = tree_get_child(tree, 1);
                tree_free(tree_get_child(tree, 0));
                tree_free(tree_get_child(tree, 2));
                tree_free_toplevel(tree);
                return result;
            }
            case 1: {
                // (F) rule
                struct Tree* x = tree_get_child(tree, 0);
                struct Tree* y = tree_get_child(tree, 1);
                struct Tree* z = tree_get_child(tree, 2);
                tree_free_toplevel(tree);
                return tree_apply(tree_apply(y, z), tree_apply(x, y));
            }
            case 2: {
                // (F) rule
                struct Tree* x = tree_get_child(tree_get_child(tree, 0), 1);
                struct Tree* w = tree_get_child(tree_get_child(tree, 0), 0);
                struct Tree* z = tree_get_child(tree, 2);
                tree_free(tree_get_child(tree, 1));
                tree_free_toplevel(tree);
                return tree_apply(tree_apply(z, w), x);
            }
            default: {
                printf(
                    "PANIC! First child of a combination has %llu children!\n",
                    tree_child_count(tree));
                exit(1);
            }
        }
    }
}
