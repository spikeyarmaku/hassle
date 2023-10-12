#include "eval.h"

#include "primop.h"

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)

// TODO fix it when term_app_level > 3

struct Term* _eval_step_3_branches(struct Term* term) {
    printf("app level: %d\n", term_app_level(term));
    if (term_app_level(term) > 2) {
        struct Term* z = term_child_right(term);
        struct Term* y = term_child_right(term_child_left(term));
        struct Term* first_apply = term_child_right(term_traverse(term, -2));
        if (term_type(first_apply) != TERM_TYPE_FORK) {
            printf("K rule\n");
            // K rule
            struct Term* result = term_copy(y);
            term_free(term);
            return result;
        } else {
            struct Term* x = term_child_right(first_apply);
            if (term_type(term_child_left(first_apply)) != TERM_TYPE_FORK) {
                printf("S rule\n");
                // S rule
                struct Term* result =
                    term_apply(term_apply(term_copy(y), term_copy(z)),
                        term_apply(term_copy(x), term_copy(z)));
                term_free(term);
                return result;
            } else {
                printf("F rule\n");
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
        struct Term* op = term_traverse(term, -term_app_level(term));
        if (term_type(op) == TERM_TYPE_PRIMOP) {
            printf("Primop\n");
            struct Term* term2 = term_child_right(term);
            struct Term* term1 = term_child_right(term_child_left(term));
            struct Term* result =
                primop_apply(term_get_primop(op), term1, term2);
            term_free(term);
            return result;
        } else {
            printf("No rules\n");
            return term;
        }
    }
}

// Check how many branches does the term have, and evaluate the 3-branch subtree
struct Term* eval_step(struct Term* term) {
    if (term_app_level(term) > 3) {
        struct Term* subterm_parent =
            term_traverse(term, -(term_app_level(term) - 4));
        struct Term* subterm = term_child_left(subterm_parent);
        struct Term* new_subterm = eval_step(subterm);
        term_set_children(subterm_parent, new_subterm,
            term_child_right(subterm_parent));
        return term;
    } else {
        // Check if the leftmost child has at most 2 branches
        struct Term* leftmost = term_traverse(term, -term_app_level(term) + 1);
        struct Term* leftmost_child = term_child_right(leftmost);
        if (term_app_level(leftmost_child) > 2) {
            struct Term* new_right = eval_step(leftmost_child);
            term_set_children(leftmost, term_child_left(leftmost), new_right);
            return term;
        } else {
            return _eval_step_3_branches(term);
        }
    }
}

struct Term* eval(struct Term* term) {
    while (!eval_done(term)) {
        term = eval_step(term);
    }
    return term;
}

BOOL eval_done(struct Term* term) {
    struct Term* leftmost = term_traverse(term, -term_app_level(term));
    return (term_app_level(term) > 2 ||
        term_type(leftmost) == TERM_TYPE_PRIMOP) ? FALSE : TRUE;
}