#include "eval.h"

#include "primop.h"

// Evaluation rules:
// ΔΔyz        = y         (K)
// Δ(Δx)yz     = yz(xz)    (S)
// Δ(Δwx)yz    = zwx       (F)

struct Term* _eval_step_3_branches(struct Term* term) {
    switch (term_child_count(term_get_child(term, 0))) {
        case 0: {
            // (K) rule
            printf("K Rule\n");
            struct Term* y = term_get_child(term, 1);
            struct Term* z = term_get_child(term, 2);
            term_free(term_get_child(term, 0));
            term_free(z);
            term_free_node(term);
            return y;
        }
        case 1: {
            // (S) rule
            printf("S Rule\n");
            struct Term* x = term_get_child(term_get_child(term, 0), 0);
            struct Term* y = term_get_child(term, 1);
            struct Term* z1 = term_get_child(term, 2);
            struct Term* z2 = term_copy(z1);
            term_free_node(term_get_child(term, 0));
            term_free_node(term);
            return term_apply(term_apply(y, z1), term_apply(x, z2));
        }
        case 2: {
            // (F) rule
            printf("F Rule\n");
            struct Term* w = term_get_child(term_get_child(term, 0), 0);
            struct Term* x = term_get_child(term_get_child(term, 0), 1);
            struct Term* y = term_get_child(term, 1);
            struct Term* z = term_get_child(term, 2);
            term_free_node(term_get_child(term, 0));
            term_free_node(term);
            term_free(y);
            return term_apply(term_apply(z, w), x);
        }
        default: {
            // Eval first child
            printf("Eval first child\n");
            struct Term* child = term_get_child(term, 0);
            struct Term* result = eval_step(child);
            term_set_child(term, 0, result);
            return term;
        }
    }
}

// Evaluate the first three branches of the tree (or first two, if it is a
// primop), or check if its first child is evaluable
struct Term* eval_step(struct Term* term) {
    uint8_t child_count = term_child_count(term);
    if (term_type(term) == TERM_TYPE_PRIMOP && child_count == 2) {
        printf("PRIMOP\n");
        struct Term* child1 = term_get_child(term, 0);
        struct Term* child2 = term_get_child(term, 1);
        struct Term* result = primop_apply(term_get_primop(term),
            child1, child2);
        term_free(term);
        return result;
    }
    else {
        if (term_child_count(term) > 3) {
            printf("More than three branches\n");
            struct Term* last = term_detach_last(term);
            struct Term* result = eval_step(term);
            return term_apply(result, last);
        } else {
            if (term_child_count(term) == 3) {
                printf("3 BRANCHES\n");
                return _eval_step_3_branches(term);
                //
            } else {
                printf("Checking first child\n");
                // Check if the leftmost child has at most 2 branches
                return eval_step(term_get_child(term, 0));
            }
        }
    }
}

struct Term* eval(struct Term* term) {
    while (!eval_done(term)) {
        term = eval_step(term);
    }
    return term;
}

BOOL eval_is_evaluable(struct Term* term) {
    uint8_t child_count = term_child_count(term);
    if (child_count > 2) {
        return TRUE;
    } else {
        return (term_type(term) == TERM_TYPE_PRIMOP && child_count == 2) ?
            TRUE : FALSE;
    }
}

// Evaluation is done iff the term is not evaluable, and its first child is also
// not evaluable (it is recursive!)
BOOL eval_done(struct Term* term) {
    if (eval_is_evaluable(term)) {
        return FALSE;
    } else {
        if (term_child_count(term) > 0) {
            return eval_done(term_get_child(term, 0));
        } else {
            return TRUE;
        }
    }
}