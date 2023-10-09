#include "combinators.h"

struct Tree* delta() {
    return tree_make(0);
}

struct Tree* cK() {
    return tree_apply(delta(), delta());
}

struct Tree* cI() {
    return tree_apply(tree_apply(delta(), cK()), cK());
}

struct Tree* cD() {
    return tree_apply(tree_apply(delta(), cK()), tree_apply(cK(), delta()));
}

// d{x} = Δ(Δx)
struct Tree* nD(struct Tree* tree) {
    return tree_apply(delta(), tree_apply(delta(), tree));
}

// S = d{K D}(d{K}(K D))
struct Tree* cS() {
    return
        tree_apply(nD(tree_apply(cK(), cD())),
            tree_apply(nD(cK()), tree_apply(cK(), cD())));
}

struct Tree* true() {
    return cK();
}

struct Tree* false() {
    return tree_apply(cK(), cI());
}

// d{K(K I)} = Δ(Δ(K(K I)))
struct Tree* and() {
    return
        tree_apply(delta(),
            tree_apply(delta(),
                tree_apply(cK(),
                    tree_apply(cK(), cI()))));
}

// d{KK}I
struct Tree* or() {
    return tree_apply(nD(tree_apply(cK(), cK())), cI());
}

// d{KK} (d{K(K I)}I)
struct Tree* not() {
    return tree_apply(nD(tree_apply(cK(), cK())),
        tree_apply(nD(tree_apply(cK(), tree_apply(cK(), cI()))), cI()));
}

struct Tree* pair() {
    return delta();
}

// first{p} = ΔpΔK
struct Tree* nFirst(struct Tree* tree) {
    return tree_apply(tree_apply(tree_apply(delta(), tree), delta()), cK());
}

// second{p} = ΔpΔ(K I)
struct Tree* nSecond(struct Tree* tree) {
    return tree_apply(tree_apply(tree_apply(delta(), tree), delta()),
        tree_apply(cK(), cI()));
}

struct Tree* nNat(size_t n) {
    return n == 0 ? delta() : tree_apply(cK(), nNat(n - 1));
}

struct Tree* nPow(struct Tree* tree, uint8_t n) {
    return n == 1 ? tree : tree_apply(tree, nPow(tree, n - 1));
}

// isZero = d{(K^4)I} (d{KK}Δ)
struct Tree* is_zero() {
    return tree_apply(nD(tree_apply(nPow(cK(), 4), cI())),
        tree_apply(nD(tree_apply(cK(), cK())), delta()));
}

struct Tree* nSucc(struct Tree* tree) {
    return tree_apply(cK(), tree);
}

// predecessor n = ΔnΔ(K I)
struct Tree* nPred(struct Tree* tree) {
    return tree_apply(tree_apply(tree_apply(delta(), tree), delta()),
        tree_apply(cK(), cI()));
}

// queries
// query{is0, is1, is2} = d{K is1}(d{K^2 I}(d{K^5 is2}(d{K^3 is0}Δ)))
struct Tree* nQuery(struct Tree* is0, struct Tree* is1, struct Tree* is2) {
    return
        tree_apply(
            nD(tree_apply(cK(), is0)),
            tree_apply(
                nD(tree_apply(nPow(cK(), 2), cI())),
                tree_apply(
                    nD(tree_apply(nPow(cK(), 5), is2)),
                    tree_apply(
                        nD(tree_apply(nPow(cK(), 3), is0)),
                        delta()))));
}

// isLeaf = query{K, K I, K I}
// isStem = query{K I, K, K I}
// isFork = query{K I, K I, K}
struct Tree* is_leaf() {
    return nQuery(cK(), tree_apply(cK(), cI()), tree_apply(cK(), cI()));
}

struct Tree* is_stem() {
    return nQuery(tree_apply(cK(), cI()), cK(), tree_apply(cK(), cI()));
}

struct Tree* is_fork() {
    return nQuery(tree_apply(cK(), cI()), tree_apply(cK(), cI()), cK());
}

// [x]x = I
// [x]y = K y (y =/= x)
// [x]O = KO
// [x]uv = d{[x]v}([x]u)
struct Tree* nBracket(char* symbol, struct Tree* tree) {
    if (tree_is_value(tree)) {
        if (tree_leaf_type(tree) == LEAF_TYPE_SYMBOL &&
            strcmp(symbol, tree_get_sym(tree)) == 0)
        {
                return cI();
        } else {
            return tree_apply(cK(), tree);
        }
    } else {
        if (tree_child_count(tree) == 0) {
            return tree_apply(cK(), tree);
        } else {
            if (tree_child_count(tree) == 1) {
                return tree_apply(cK(), tree);
            } else {
                return tree_apply(nD(nBracket(symbol, tree_get_child(tree, 1))),
                    nBracket(symbol, tree_get_child(tree, 0)));
            }
        }
    }
}

BOOL is_elem(char* symbol, struct Tree* tree) {
    if (tree_is_value(tree)) {
        if (tree_leaf_type(tree) == LEAF_TYPE_SYMBOL &&
            strcmp(symbol, tree_get_str(tree)) == 0)
        {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if (tree_child_count(tree) == 0) {
            return FALSE;
        } else {
            for (uint8_t i = 0; i < tree_child_count(tree); i++) {
                if (is_elem(symbol, tree_get_child(tree, i)) == TRUE) {
                    return TRUE;
                }
            }
            return FALSE;
        }
    }
}

// λ∗ x.t = Kt (x not in t)
// λ∗ x.t x = t (x not in t)
// λ∗ x.x = I
// λ∗ x.tu = d{λ∗ x.u}(λ∗ x.t) (otherwise).
struct Tree* nStar(char* symbol, struct Tree* tree) {
    if (is_elem(symbol, tree) == FALSE) {
        return tree_apply(cK(), tree);
    } else {
        if (tree_is_symbol(symbol, tree) == TRUE) {
            return cI();
        } else {
            // If last child is the symbol
            struct Tree* last_child =
                tree_get_child(tree, tree_child_count(tree) - 1);
            // Create a new tree without the last child
            uint8_t new_child_count = tree_child_count(tree) - 1;
            struct Tree* new_tree = tree_make(new_child_count);
            for (uint8_t i = 0; i < new_child_count; i++) {
                tree_set_child(new_tree, i, tree_get_child(tree, i));
            }
            // Delete the last child
            tree_free_toplevel(tree);
            if (tree_is_symbol(symbol, last_child) == TRUE) {
                tree_free(last_child);
                return new_tree;
            } else {
                return tree_apply(nD(nStar(symbol, last_child)),
                    nStar(symbol, new_tree));
            }
        }
    }
}

// wait{x, y} = d{I}(d{K y}(K x))
struct Tree* nWait(struct Tree* tree1, struct Tree* tree2) {
    return tree_apply(nD(cI()), tree_apply(nD(tree_apply(cK(), tree2)),
        tree_apply(cK(), tree1)));
}

// wait1{x} = d{d{K(K x)} (d{d{K}(KΔ)} (KΔ))}   (K(d{ΔKK}))
struct Tree* nWait1(struct Tree* tree) {
    return
        tree_apply(
            nD(tree_apply(
                nD(tree_apply(cK(), tree_apply(cK(), tree))),
                tree_apply(
                    nD(tree_apply(nD(cK()), tree_apply(cK(), delta()))),
                    tree_apply(cK(), delta())))),
            tree_apply(cK(), nD(tree_apply(tree_apply(delta(), cK()), cK()))));
}

// self_apply = λ∗w.ww = d{I}I
struct Tree* self_apply() {
    return tree_apply(nD(cI()), cI());
}

// Z { f } = wait{self_apply, d{wait1{self_apply}(K f )}
struct Tree* nZ(struct Tree* tree) {
    return nWait(self_apply(), tree_apply(nD(nWait1(self_apply())),
        tree_apply(cK(), tree)));
}

// swap{ f } = d{K f}d{d{K}(KΔ)}(KΔ)
struct Tree* nSwap(struct Tree* tree) {
    return
        tree_apply(
            tree_apply(
                nD(tree_apply(cK(), tree)),
                nD(tree_apply(nD(cK()), tree_apply(cK(), delta())))),
            tree_apply(cK(), delta()));
}

// Y2 { f } = Z {swap{ f }}
struct Tree* nY2(struct Tree* tree) {
    return nZ(nSwap(tree));
}
