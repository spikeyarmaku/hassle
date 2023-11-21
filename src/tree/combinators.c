#include "combinators.h"

// Helper function for making references
struct Tree* _ref(char* reference) {
    return
        tree_make_program(program_with_label(reference, program_make_leaf()));
}

// struct Tree* _str(char* string) {
//     return tree_make_program(program_make_value(value_make_str(string)));
// }

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
        tree_make_apply(
            tree_make_apply(tree_make_apply(delta(), tree), delta()), cK());
}

// second{p} = ΔpΔ(K I)
struct Tree* nSecond(struct Tree* tree) {
    return
        tree_make_apply(
            tree_make_apply(tree_make_apply(delta(), tree), delta()),
            tree_make_apply(cK(), cI()));
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
struct Tree* nBracket(char* reference, struct Tree* tree) {
    // Ref y => if eqb x y then I else (K@ (Ref y))
    if (tree_is_reference(tree) == TRUE) {
        if (strcmp(reference, tree_get_label(tree)) == 0) {
            tree_free(tree);
            return cI();
        } else {
            return tree_make_apply(cK(), tree);
        }
    } else {
        struct Tree* extracted = tree_extract(tree);
        // △ => K@△
        if (tree_get_type(extracted) == TREE_TYPE_PROGRAM) {
            return tree_make_apply(cK(), tree);
        }

        struct Tree* child0 = tree_copy(tree_get_apply(extracted, 0));
        struct Tree* child1 = tree_copy(tree_get_apply(extracted, 1));
        tree_free(extracted);

        // App M1 M2 => d (bracket x M2) @ (bracket x M1)
        return
            tree_make_apply(
                nD(nBracket(reference, child1)), nBracket(reference, child0));
    }
}

BOOL occurs_t(char* reference, struct Tree* tree) {
    if (tree == NULL) {
        return FALSE;
    }

    if (tree_get_type(tree) == TREE_TYPE_PROGRAM) {
        return occurs_p(reference, tree_get_program(tree));
    } else {
        return
            occurs_t(reference, tree_get_apply(tree, 0)) ||
            occurs_t(reference, tree_get_apply(tree, 1));
    }
}

BOOL occurs_p(char* reference, struct Program* prg) {
    if (prg == NULL) {
        return FALSE;
    }

    if (program_get_type(prg) == PROGRAM_TYPE_LEAF) {
        if (program_get_label(prg) != NULL) {
            if (strcmp(program_get_label(prg), reference) == 0) {
                return TRUE;
            }
        }
        return FALSE;
    } else {
        return
            occurs_p(reference, program_get_child(prg, 0)) ||
            occurs_p(reference, program_get_child(prg, 1));
    }
}

// λ∗x.t = Kt (x not in t)
// λ∗x.t x = t (x not in t)
// λ∗x.x = I
// λ∗x.tu = d{λ∗x.u}(λ∗x.t) (otherwise).
struct Tree* nStar(char* reference, struct Tree* tree) {
    // Ref y => if eqb x y then I else (K@ (Ref y))
    if (tree_is_reference(tree) == TRUE) {
        if (strcmp(reference, tree_get_label(tree)) == 0) {
            tree_free(tree);
            return cI();
        } else {
            return tree_make_apply(cK(), tree);
        }
    }

    struct Tree* extracted = tree_extract(tree);
    // △ => K@△
    if (tree_get_type(extracted) == TREE_TYPE_PROGRAM) {
        return tree_make_apply(cK(), tree);
    }

    struct Tree* child0 = tree_copy(tree_get_apply(extracted, 0));
    struct Tree* child1 = tree_copy(tree_get_apply(extracted, 1));
    tree_free(extracted);

    if (tree_is_reference(child1) == TRUE) {
        // App M1 (Ref y) => ...
        BOOL x_in_child_0 = occurs_t(reference, child0);
        if (strcmp(reference, tree_get_label(child1)) == 0) {
            tree_free(child1);
            if (x_in_child_0 == TRUE) {
                return tree_make_apply(nD(cI()), nStar(reference, child0));
            } else {
                return child0;
            }
        } else {
            if (x_in_child_0) {
                // d (K@ (Ref y)) @ (star x M1)
                return
                    tree_make_apply(
                        nD(tree_make_apply(cK(), child1)),
                        nStar(reference, child0));
            } else {
                // K@ (M1 @ (Ref y))
                return tree_make_apply(cK(), tree_make_apply(child0, child1));
            }
        }
    } else {
        // App M1 M2 => ...
        if (occurs_t(reference, child0) || occurs_t(reference, child1)) {
            // Node @ (Node @ (star x M2)) @ (star x M1)
            return
                tree_make_apply(
                    nD(nStar(reference, child1)), nStar(reference, child0));
        } else {
            // K@ (M1 @ M2)
            return tree_make_apply(cK(), tree_make_apply(child0, child1));
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
    return nStar("x", tree_make_apply(_ref("x"), _ref("x")));
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

// get_tag = λ∗p.first{first{p}Δ}
struct Tree* get_tag() {
    return nStar("p", nFirst(tree_make_apply(nFirst(_ref("p")), delta())));
}

// tag_wait{t} = λ∗w.tag{t, wait{self_apply, w}} (w not in t).
struct Tree* nTagWait(struct Tree* tree) {
    return nStar("w", nTag(tree, nWait(self_apply(), _ref("w"))));
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
        nStar("a", nStar("y", nStar("z", _ref("z"))));
}

// in the book:   successor_rule = λ∗x.λ∗a.λ∗y.λ∗z.yx
// in the proofs: successor_rule = λ∗x.λ∗a.λ∗y.[z].yx
struct Tree* successor_rule() {
    return
        nStar("x",
            nStar("a",
                nStar("y",
                    nBracket("z", tree_make_apply(_ref("y"), _ref("x"))))));
    // return
    //     nStar("x",
    //         nStar("a",
    //             nStar("y",
    //                 nStar("z", tree_apply(_ref("y"), _ref("x"))))));
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
                            tree_make_apply(
                                tree_make_apply(
                                    tree_make_apply(
                                        tree_make_apply(
                                            _ref("a"),
                                            _ref("w")),
                                        _ref("y")),
                                    _ref("z")),
                                tree_make_apply(
                                    tree_make_apply(
                                        tree_make_apply(
                                            _ref("a"),
                                            _ref("x")),
                                        _ref("y")),
                                    _ref("z"))))))));
    // return
    //     nStar("w",
    //         nStar("x",
    //             nStar("a",
    //                 nStar("y",
    //                     nStar("z",
    //                         tree_apply(_ref("y"), _ref("x")))))));
}

// empty_rule = λ∗a.λ∗y.λ∗z.a
struct Tree* empty_rule() {
    return
        nStar("a", nStar("y", nStar("z", _ref("a"))));
}

// substitution_rule = λ∗x.λ∗a.λ∗y.λ∗z.azxy
struct Tree* substitution_rule() {
    struct Tree* tree =
        tree_make_apply(
            tree_make_apply(
                tree_make_apply(
                    _ref("a"),
                    _ref("z")),
                _ref("x")),
            _ref("y"));
    return nStar("x", nStar("a", nStar("y", nStar("z", tree))));
}

// abstraction_rule = λ∗w.λ∗x.λ∗a.λ∗y.λ∗z.aw(axyz)
struct Tree* abstraction_rule() {
    return
        nStar("w", nStar("x", nStar("a", nStar("y", nStar("z",
            tree_make_apply(
                tree_make_apply(
                    _ref("a"),
                    _ref("w")),
                tree_make_apply(
                    tree_make_apply(
                        tree_make_apply(
                            _ref("a"),
                            _ref("x")),
                        _ref("y")),
                    _ref("z"))))))));
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
                        tree_make_apply(successor_rule(), _ref("x")),
                        nStar("y",
                            nTag(
                                tree_make_apply(
                                    tree_make_apply(
                                        application_rule(), _ref("x")),
                                    _ref("y")),
                                nBracket("z",
                                    tree_make_apply(
                                        tree_make_apply(
                                            _ref("a"),
                                            tree_make_apply(
                                                tree_make_apply(
                                                    _ref("a"),
                                                    _ref("x")),
                                                _ref("y"))),
                                        _ref("z")))))))));
}

// At = Y2t {empty_rule,
//      λ∗x.λ∗a.tag{substitution_rule x,
//      λ∗y.tag{abstraction_rule x y,
//          get_tag x a y}}}
struct Tree* cA() {
    return
        nY2t(empty_rule(),
            nStar("x",
                nStar("a",
                    nTag(
                        tree_make_apply(substitution_rule(), _ref("x")),
                        nStar("y",
                            nTag(
                                tree_make_apply(
                                    tree_make_apply(
                                        abstraction_rule(), _ref("x")),
                                    _ref("y")),
                                tree_make_apply(
                                    tree_make_apply(
                                        tree_make_apply(get_tag(), _ref("x")),
                                        _ref("a")),
                                    _ref("y"))))))));
}
