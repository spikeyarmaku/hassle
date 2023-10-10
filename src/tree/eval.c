#include "eval.h"

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)

struct Term* eval_step(struct Term* term) {
    if (term_app_level(term) > 2) {
        struct Term* z = term_child_right(term);
        struct Term* y = term_child_right(term_child_left(term));
        struct Term* first_apply = term_child_right(term_traverse(term, -2));
        if (term_type(first_apply) != TERM_TYPE_FORK) {
            // K rule
            struct Term* result = term_copy(y);
            term_free(term);
            return result;
        } else {
            struct Term* x = term_child_right(first_apply);
            if (term_type(term_child_left(term_child_right(first_apply))) !=
                TERM_TYPE_FORK)
            {
                // S rule
                struct Term* result =
                    term_apply(term_apply(term_copy(y), term_copy(z)),
                        term_apply(term_copy(x), term_copy(z)));
                term_free(term);
                return result;
            } else {
                // F rule
                struct Term* w = term_child_right(term_child_left(first_apply));
                struct Term* result =
                    term_apply(term_apply(term_copy(z), term_copy(w)),
                        term_copy(x));
                term_free(term);
                return result;
            }
        }
    } else {
        return term;
    }
}

struct Term* eval(struct Term* term) {
    while (term_app_level(term) > 2) {
        term = eval_step(term);
    }
    return term;
}