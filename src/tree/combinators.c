#include "combinators.h"

// Helper function for making symbols
struct Tree* _sym(char* symbol) {
    return tree_make_program(program_make_value(value_make_sym(symbol)));
}

struct Tree* delta() {
    return tree_make_program(program_make_leaf());
}

struct Tree* cK() {
    return tree_apply(delta(), delta());
}

struct Tree* cI() {
    return tree_apply(tree_apply(delta(), cK()), cK());
}

struct Tree* cD() {
    return
        tree_apply(
            tree_apply(delta(), cK()),
            tree_apply(delta(), cK()));
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
        tree_apply(nD(tree_apply(cK(), tree_apply(cK(), cI()))),
            cI()));
}

struct Tree* pair() {
    return delta();
}

// first{p} = ΔpΔK
struct Tree* nFirst(struct Tree* tree) {
    return
        tree_apply(
            tree_apply(tree_apply(delta(), tree), delta()), cK());
}

// second{p} = ΔpΔ(K I)
struct Tree* nSecond(struct Tree* tree) {
    return
        tree_apply(
            tree_apply(tree_apply(delta(), tree), delta()),
            tree_apply(cK(), cI()));
}

// d(KK)(d(KΔ)Δ)
struct Tree* fst() {
    return
        tree_apply(
            nD(tree_apply(cK(), cK())),
            tree_apply(nD(tree_apply(cK(), delta())), delta()));
}

// d(K(KI))(d(KΔ)Δ)
struct Tree* snd() {
    return
        tree_apply(
            nD(tree_apply(cK(), tree_apply(cK(), cI()))),
            tree_apply(nD(tree_apply(cK(), delta())), delta()));
}

struct Tree* nNat(size_t n) {
    return n == 0 ? delta() : tree_apply(cK(), nNat(n - 1));
}

struct Tree* nPow(struct Tree* tree, uint8_t n) {
    return n == 1 ? tree : tree_apply(tree, nPow(tree, n - 1));
}

// isZero = d{(K^4)I} (d{KK}Δ)
struct Tree* is_zero() {
    return
        tree_apply(
            nD(tree_apply(nPow(cK(), 4), cI())),
            tree_apply(nD(tree_apply(cK(), cK())), delta()));
}

struct Tree* nSucc(struct Tree* tree) {
    return tree_apply(cK(), tree);
}

// predecessor n = ΔnΔ(K I)
struct Tree* nPred(struct Tree* tree) {
    return
        tree_apply(
            tree_apply(tree_apply(delta(), tree), delta()),
            tree_apply(cK(), cI()));
}

// queries
// query{is0, is1, is2} = d{K is1}(d{K^2 I}(d{K^5 is2}(d{K^3 is0}Δ)))
struct Tree* nQuery(struct Tree* is0, struct Tree* is1, struct Tree* is2) {
    return
        tree_apply(
            nD(tree_apply(cK(), is1)),
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
    // printf("nbracket %s\n", symbol);
    if (tree_get_type(tree) == TREE_TYPE_PROGRAM) {
        // printf("tree type: VALUE\n");
        struct Value* val = program_get_value(tree_get_program(tree));
        if (value_get_type(val) == VALUE_TYPE_SYMBOL) {
            // printf("value type: SYMBOL\n");
            if (strcmp(value_get_sym(val), symbol) == 0) {
                // printf("symbol match %s\n", symbol);
                tree_free(tree);
                return cI();
            } else {
                // printf("symbol NOT match %s\n", symbol);
                return tree_apply(cK(), tree);
            }
        } else {
            // printf("value type: %d\n", value_get_type(val));
            return tree_apply(cK(), tree);
        }
    } else {
        // printf("tree type: APPLY\n");
        struct Tree* app0 = tree_copy(tree_get_apply(tree, 0));
        struct Tree* app1 = tree_copy(tree_get_apply(tree, 1));
        tree_free(tree);
        return
            tree_apply(nD(nBracket(symbol, app1)), nBracket(symbol, app0));
    }
}

BOOL is_elem(char* symbol, struct Tree* tree) {
    if (tree_get_type(tree) == TREE_TYPE_PROGRAM) {
        struct Value* val = program_get_value(tree_get_program(tree));
        if (value_get_type(val) == VALUE_TYPE_SYMBOL) {
            if (strcmp(value_get_sym(val), symbol) == 0) {
                return TRUE;
            } else {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    } else {
        return
            is_elem(symbol, tree_get_apply(tree, 0)) ||
            is_elem(symbol, tree_get_apply(tree, 1));
    }
}

// λ∗x.t = Kt (x not in t)
// λ∗x.t x = t (x not in t)
// λ∗x.x = I
// λ∗x.tu = d{λ∗x.u}(λ∗x.t) (otherwise).
struct Tree* nStar(char* symbol, struct Tree* tree) {
    if (tree_get_type(tree) == TREE_TYPE_PROGRAM) {
        struct Value* val = program_get_value(tree_get_program(tree));
        if (value_get_type(val) == VALUE_TYPE_SYMBOL &&
            strcmp(value_get_sym(val), symbol) == 0)
        {
            // λ∗ x.x = I
            tree_free(tree);
            return cI();
        } else {
            // λ∗ x.t = Kt (x not in t)
            return tree_apply(cK(), tree);
        }
    } else {
        struct Tree* child1 = tree_get_apply(tree, 1);
        struct Value* val = NULL;
        if (tree_get_type(child1) == TREE_TYPE_PROGRAM) {
            val = program_get_value(tree_get_program(child1));
        }
        if (val != NULL &&
            value_get_type(val) == VALUE_TYPE_SYMBOL &&
            strcmp(value_get_sym(val), symbol) == 0 &&
            !is_elem(symbol, tree_get_apply(tree, 0)))
        {
            // λ∗ x.t x = t (x not in t)
            struct Tree* result = tree_copy(tree_get_apply(tree, 0));
            tree_free(tree);
            return result;
        } else {
            // λ∗ x.tu = d{λ∗ x.u}(λ∗ x.t)
            struct Tree* app0 = tree_copy(tree_get_apply(tree, 0));
            struct Tree* app1 = tree_copy(tree_get_apply(tree, 1));
            tree_free(tree);
            return tree_apply(nD(nStar(symbol, app1)), nStar(symbol, app0));
        }
    }
}

// wait{x, y} = d{I}(d{Ky}(Kx))
struct Tree* nWait(struct Tree* term1, struct Tree* term2) {
    return
        tree_apply(
            nD(cI()),
            tree_apply(nD(tree_apply(cK(), term2)),
                tree_apply(cK(), term1)));
}

// wait1{x} = d{d{K(K x)} (d{d{K}(KΔ)} (KΔ))} (K(d{ΔKK}))
struct Tree* nWait1(struct Tree* tree) {
    return
        tree_apply(
            nD(tree_apply(
                nD(tree_apply(cK(), tree_apply(cK(), tree))),
                tree_apply(
                    nD(tree_apply(nD(cK()),
                        tree_apply(cK(), delta()))),
                    tree_apply(cK(), delta())))),
            tree_apply(cK(),
                nD(tree_apply(tree_apply(delta(), cK()), cK()))));
}

// in the book:   self_apply = λ∗w.ww = d{I}I
// in the proofs: self_apply = \*x.xx
struct Tree* self_apply() {
    return nStar("x", tree_make_apply(_sym("x"), _sym("x")));
    // return tree_apply(nD(cI()), cI());
}

// Z{f} = wait{self_apply, d{wait1{self_apply}(Kf)}
struct Tree* nZ(struct Tree* tree) {
    return
        nWait(
            self_apply(),
            tree_apply(
                nD(nWait1(self_apply())),
                tree_apply(cK(), tree)));
}

// swap{f} = d{Kf}(d{d{K}(KΔ)}(KΔ))
struct Tree* nSwap(struct Tree* tree) {
    return
        tree_apply(
            nD(tree_apply(cK(), tree)),
            tree_apply(
                nD(tree_apply(nD(cK()), tree_apply(cK(), delta()))),
                tree_apply(cK(), delta())));
}

// Y2{f} = Z{swap{f}}
struct Tree* nY2(struct Tree* tree) {
    return nZ(nSwap(tree));
}

// tag{t, f} = d{t}(d{f}(KK))
struct Tree* nTag(struct Tree* tag, struct Tree* tree) {
    return
        tree_apply(
            nD(tag), tree_apply(nD(tree), tree_apply(cK(), cK())));
}

// getTag = λ∗p.first{first{p}Δ}
struct Tree* getTag() {
    return nStar("p", nFirst(tree_apply(nFirst(_sym("p")), delta())));
}

// tag_wait{t} = λ∗w.tag{t, wait{self_apply, w}} (w not in t).
struct Tree* nTagWait(struct Tree* tree) {
    return nStar("w", nTag(tree, nWait(self_apply(), _sym("w"))));
}

// in the book:   Y2t{t, f} = tag{t, wait{self_apply, d{tag_wait{t}(Kf)}}
// in the proofs: Y2t{t, f} = tag{t, wait{self_apply, d{tag_wait{t}}(K (swap f))}
struct Tree* nY2t(struct Tree* tag, struct Tree* tree) {
    struct Tree* tag0 = tag;
    struct Tree* tag1 = tree_copy(tag);
    return
        nTag(tag0,
            nWait(self_apply(),
                tree_apply(
                    nD(nTagWait(tag1)),
                    tree_apply(cK(), nSwap(tree)))));
}

// zero_rule = λ∗a.λ∗y.λ∗z.z
struct Tree* zero_rule() {
    return
        nStar("a", nStar("y", nStar("z", _sym("z"))));
}

// in the book:   successor_rule = λ∗x.λ∗a.λ∗y.λ∗z.yx
// in the proofs: successor_rule = λ∗x.λ∗a.λ∗y.[z].yx
struct Tree* successor_rule() {
    return
        nStar("x",
            nStar("a",
                nStar("y",
                    nBracket("z", tree_apply(_sym("y"), _sym("x"))))));
    // return
    //     nStar("x",
    //         nStar("a",
    //             nStar("y",
    //                 nStar("z", tree_apply(_sym("y"), _sym("x"))))));
}

// in the book:   application_rule = λ∗w.λ∗x.λ∗a.λ∗y.λ∗z.yx
// in the proofs: application_rule = λ∗w.λ∗x.λ∗a.λ∗y.λ∗z.awyz(axyz)
struct Tree* application_rule() {
    return
        nStar("w",
            nStar("x",
                nStar("a",
                    nStar("y",
                        nStar("z",
                            tree_apply(
                                tree_apply(
                                    tree_apply(
                                        tree_apply(
                                            _sym("a"),
                                            _sym("w")),
                                        _sym("y")),
                                    _sym("z")),
                                tree_apply(
                                    tree_apply(
                                        tree_apply(
                                            _sym("a"),
                                            _sym("x")),
                                        _sym("y")),
                                    _sym("z"))))))));
    // return
    //     nStar("w",
    //         nStar("x",
    //             nStar("a",
    //                 nStar("y",
    //                     nStar("z",
    //                         tree_apply(_sym("y"), _sym("x")))))));
}

// empty_rule = λ∗a.λ∗y.λ∗z.a
struct Tree* empty_rule() {
    return
        nStar("a", nStar("y", nStar("z", _sym("a"))));
}

// substitution_rule = λ∗x.λ∗a.λ∗y.λ∗z.azxy
struct Tree* substitution_rule() {
    struct Tree* tree =
        tree_apply(tree_apply(tree_apply(_sym("a"), _sym("z")), _sym("x")),
            _sym("y"));
    return nStar("x", nStar("a", nStar("y", nStar("z", tree))));
}

// abstraction_rule = λ∗w.λ∗x.λ∗a.λ∗y.λ∗z.aw(axyz)
struct Tree* abstraction_rule() {
    return
        nStar("w", nStar("x", nStar("a", nStar("y", nStar("z",
            tree_apply(
                tree_apply(
                    _sym("a"),
                    _sym("w")),
                tree_apply(
                    tree_apply(
                        tree_apply(
                            _sym("a"),
                            _sym("x")),
                        _sym("y")),
                    _sym("z"))))))));
}

// Vt = Y2t {zero_rule,
//      λ∗x.λ∗a.tag{successor_rule x,
//      λ∗y.tag{application_rule x y,
//          [z](a(axy)z)}}}
struct Tree* cV() {
    return
        nY2t(zero_rule(),
            nStar("x",
                nStar("a",
                    nTag(
                        tree_apply(successor_rule(),
                            _sym("x")),
                        nStar("y",
                            nTag(
                                tree_apply(
                                    tree_apply(
                                        application_rule(),
                                        _sym("x")),
                                    _sym("y")),
                                nBracket("z",
                                    tree_apply(
                                        tree_apply(
                                            _sym("a"),
                                            tree_apply(
                                                tree_apply(
                                                    _sym("a"),
                                                    _sym("x")),
                                                _sym("y"))),
                                        _sym("z")))))))));
}

// At = Y2t {empty_rule,
//      λ∗x.λ∗a.tag{substitution_rule x,
//      λ∗y.tag{abstraction_rule x y,
//          getTag x a y}}}
struct Tree* cA() {
    return
        nY2t(empty_rule(),
            nStar("x",
                nStar("a",
                    nTag(
                        tree_apply(substitution_rule(),
                            _sym("x")),
                        nStar("y",
                            nTag(
                                tree_apply(
                                    tree_apply(
                                        abstraction_rule(),
                                        _sym("x")),
                                    _sym("y")),
                                tree_apply(
                                    tree_apply(
                                        tree_apply(
                                            getTag(),
                                            _sym("x")),
                                        _sym("a")),
                                    _sym("y"))))))));
}
