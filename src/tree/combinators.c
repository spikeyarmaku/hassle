#include "combinators.h"

struct Term* delta() {
    return term_make_node();
}

struct Term* cK() {
    return term_apply(delta(), delta());
}

struct Term* cI() {
    return term_apply(term_apply(delta(), cK()), cK());
}

struct Term* cD() {
    return term_apply(term_apply(delta(), cK()), term_apply(cK(), delta()));
}

// d{x} = Δ(Δx)
struct Term* nD(struct Term* term) {
    return term_apply(delta(), term_apply(delta(), term));
}

// S = d{K D}(d{K}(K D))
struct Term* cS() {
    return
        term_apply(nD(term_apply(cK(), cD())),
            term_apply(nD(cK()), term_apply(cK(), cD())));
}

struct Term* true() {
    return cK();
}

struct Term* false() {
    return term_apply(cK(), cI());
}

// d{K(K I)} = Δ(Δ(K(K I)))
struct Term* and() {
    return
        term_apply(delta(),
            term_apply(delta(),
                term_apply(cK(),
                    term_apply(cK(), cI()))));
}

// d{KK}I
struct Term* or() {
    return term_apply(nD(term_apply(cK(), cK())), cI());
}

// d{KK} (d{K(K I)}I)
struct Term* not() {
    return term_apply(nD(term_apply(cK(), cK())),
        term_apply(nD(term_apply(cK(), term_apply(cK(), cI()))), cI()));
}

struct Term* pair() {
    return delta();
}

// first{p} = ΔpΔK
struct Term* nFirst(struct Term* term) {
    return term_apply(term_apply(term_apply(delta(), term), delta()), cK());
}

// second{p} = ΔpΔ(K I)
struct Term* nSecond(struct Term* term) {
    return term_apply(term_apply(term_apply(delta(), term), delta()),
        term_apply(cK(), cI()));
}

struct Term* nNat(size_t n) {
    return n == 0 ? delta() : term_apply(cK(), nNat(n - 1));
}

struct Term* nPow(struct Term* term, uint8_t n) {
    return n == 1 ? term : term_apply(term, nPow(term, n - 1));
}

// isZero = d{(K^4)I} (d{KK}Δ)
struct Term* is_zero() {
    return term_apply(nD(term_apply(nPow(cK(), 4), cI())),
        term_apply(nD(term_apply(cK(), cK())), delta()));
}

struct Term* nSucc(struct Term* term) {
    return term_apply(cK(), term);
}

// predecessor n = ΔnΔ(K I)
struct Term* nPred(struct Term* term) {
    return term_apply(term_apply(term_apply(delta(), term), delta()),
        term_apply(cK(), cI()));
}

// queries
// query{is0, is1, is2} = d{K is1}(d{K^2 I}(d{K^5 is2}(d{K^3 is0}Δ)))
struct Term* nQuery(struct Term* is0, struct Term* is1, struct Term* is2) {
    return
        term_apply(
            nD(term_apply(cK(), is0)),
            term_apply(
                nD(term_apply(nPow(cK(), 2), cI())),
                term_apply(
                    nD(term_apply(nPow(cK(), 5), is2)),
                    term_apply(
                        nD(term_apply(nPow(cK(), 3), is0)),
                        delta()))));
}

// isLeaf = query{K, K I, K I}
// isStem = query{K I, K, K I}
// isFork = query{K I, K I, K}
struct Term* is_leaf() {
    return nQuery(cK(), term_apply(cK(), cI()), term_apply(cK(), cI()));
}

struct Term* is_stem() {
    return nQuery(term_apply(cK(), cI()), cK(), term_apply(cK(), cI()));
}

struct Term* is_fork() {
    return nQuery(term_apply(cK(), cI()), term_apply(cK(), cI()), cK());
}

// [x]x = I
// [x]y = K y (y =/= x)
// [x]O = KO
// [x]uv = d{[x]v}([x]u)
struct Term* nBracket(char* symbol, struct Term* term) {
    if (term_type(term) == TERM_TYPE_SYMBOL) {
        if (term_is_symbol(symbol, term) == TRUE) {
            term_free(term);
            return cI();
        } else {
            return term_apply(cK(), term);
        }
    } else {
        if (term_child_count(term) > 0) {
            struct Term* v = term_detach_last(term);
            struct Term* u = term;
            return term_apply(nD(nBracket(symbol, v)), nBracket(symbol, u));
        } else {
            return term_apply(cK(), term);
        }
    }
}

BOOL is_elem(char* symbol, struct Term* term) {
    if (term_type(term) == TERM_TYPE_SYMBOL &&
        strcmp(symbol, term_get_sym(term)) == 0)
    {
        return TRUE;
    } else {
        uint8_t child_count = term_child_count(term);
        for (uint8_t i = 0; i < child_count; i++) {
            if (is_elem(symbol, term_get_child(term, i)) == TRUE) {
                return TRUE;
            }
        }
        return FALSE;
    }
}

// λ∗ x.t = Kt (x not in t)
// λ∗ x.t x = t (x not in t)
// λ∗ x.x = I
// λ∗ x.tu = d{λ∗ x.u}(λ∗ x.t) (otherwise).
struct Term* nStar(char* symbol, struct Term* term) {
    // printf("nStar %s - ", symbol); term_print(term); printf("\n");
    if (is_elem(symbol, term) == FALSE) {
        // printf("\\x.t = Kt\n");
        return term_apply(cK(), term);
    } else {
        if (term_type(term) == TERM_TYPE_SYMBOL && term_is_symbol(symbol, term))
        {
            // printf("\\x.x = I\n");
            term_free(term);
            return cI();
        } else {
            struct Term* last = term_detach_last(term);
            if (term_type(last) == TERM_TYPE_SYMBOL &&
                term_is_symbol(symbol, last))
            {
                // printf("\\x.t x = t\n");
                term_free(last);
                return term;
            } else {
                // printf("\\x.tu = d{\\x.u}(\\x.t)\n");
                return term_apply(nD(nStar(symbol, last)), nStar(symbol, term));
            }
        }
    }
}

// wait{x, y} = d{I}(d{K y}(K x))
struct Term* nWait(struct Term* term1, struct Term* term2) {
    return term_apply(nD(cI()), term_apply(nD(term_apply(cK(), term2)),
        term_apply(cK(), term1)));
}

// wait1{x} = d{d{K(K x)} (d{d{K}(KΔ)} (KΔ))}   (K(d{ΔKK}))
struct Term* nWait1(struct Term* term) {
    return
        term_apply(
            nD(term_apply(
                nD(term_apply(cK(), term_apply(cK(), term))),
                term_apply(
                    nD(term_apply(nD(cK()), term_apply(cK(), delta()))),
                    term_apply(cK(), delta())))),
            term_apply(cK(), nD(term_apply(term_apply(delta(), cK()), cK()))));
}

// self_apply = λ∗w.ww = d{I}I
struct Term* self_apply() {
    return term_apply(nD(cI()), cI());
}

// Z { f } = wait{self_apply, d{wait1{self_apply}(K f )}
struct Term* nZ(struct Term* term) {
    return nWait(self_apply(), term_apply(nD(nWait1(self_apply())),
        term_apply(cK(), term)));
}

// swap{ f } = d{K f}d{d{K}(KΔ)}(KΔ)
struct Term* nSwap(struct Term* term) {
    return
        term_apply(
            term_apply(
                nD(term_apply(cK(), term)),
                nD(term_apply(nD(cK()), term_apply(cK(), delta())))),
            term_apply(cK(), delta()));
}

// Y2 { f } = Z {swap{ f }}
struct Term* nY2(struct Term* term) {
    return nZ(nSwap(term));
}
