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
            printf("K Rule");
            struct Term* y = term_get_child(term, 1);
            term_free(term_get_child(term, 0));
            term_free(term_get_child(term, 2));
            term_free_node(term);
            printf(".\n");
            return y;
        }
        case 1: {
            // (S) rule
            printf("S Rule");
            struct Term* x = term_get_child(term_get_child(term, 0), 0);
            struct Term* y = term_get_child(term, 1);
            struct Term* z1 = term_get_child(term, 2);
            struct Term* z2 = term_copy(z1);
            term_free_node(term_get_child(term, 0));
            term_free_node(term);
            printf(".\n");
            return term_apply(term_apply(y, z1), term_apply(x, z2));
        }
        case 2: {
            // (F) rule
            printf("F Rule");
            struct Term* w = term_get_child(term_get_child(term, 0), 0);
            struct Term* x = term_get_child(term_get_child(term, 0), 1);
            struct Term* y = term_get_child(term, 1);
            struct Term* z = term_get_child(term, 2);
            term_free_node(term_get_child(term, 0));
            term_free_node(term);
            term_free(y);
            printf(".\n");
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

struct Term* _eval_primop(struct Term* term) {
    struct Term* child1 = term_get_child(term, 0);
    struct Term* child2 = term_get_child(term, 1);
    struct Term* result = primop_apply(term_get_primop(term),
        child1, child2);
    term_free(term);
    return result;
}

// Evaluate the first three branches of the tree (or first two, if it is a
// primop), or check if its first child is evaluable
// If no evaluation could take place, return with NULL
struct Term* eval_step(struct Term* term) {
    // Check if it is an evaluable primop
    uint8_t child_count = term_child_count(term);
    // printf("child count: %d\n", child_count);
    BOOL is_primop_eval = FALSE;
    if (term_type(term) == TERM_TYPE_PRIMOP && child_count == 2) {
        is_primop_eval = TRUE;
        for (uint8_t i = 0; i < 2; i++) {
            uint8_t type = term_type(term_get_child(term, i));
            if ((type != TERM_TYPE_STRING) && (type != TERM_TYPE_RATIONAL)) {
                is_primop_eval = FALSE;
                break;
            }
        }
    }

    if (is_primop_eval == TRUE) {
        // printf("Primop with two primitive children\n");
        return _eval_primop(term);
    }
    else {
        // Check if it has at least 3 branches
        if (term_child_count(term) > 3) {
            // printf("More than three branches\n");
            struct Term* last = term_detach_last(term);
            struct Term* result = eval_step(term);
            return term_apply(result, last);
        } else {
            if (term_child_count(term) == 3) {
                // printf("Exactly three branches\n");
                return _eval_step_3_branches(term);
            } else {
                // printf("Less than three branches, checking children\n");
                for (uint8_t i = 0; i < term_child_count(term); i++) {
                    // Check if the leftmost child has at most 2 branches
                    struct Term* evaled_child =
                        eval_step(term_get_child(term, i));
                    if (evaled_child != NULL) {
                        term_set_child(term, i, evaled_child);
                        return term;
                    } else {
                    }
                }
                return NULL;
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
        if (term_type(term) == TERM_TYPE_PRIMOP && child_count == 2) {
            for (uint8_t i = 0; i < 2; i++) {
                uint8_t type = term_type(term_get_child(term, i));
                if (type != TERM_TYPE_STRING && type != TERM_TYPE_RATIONAL) {
                    return FALSE;
                }
            }
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

// Evaluation is done iff the term is not evaluable, and its first child is also
// not evaluable (it is recursive!)
BOOL eval_done(struct Term* term) {
    if (eval_is_evaluable(term)) {
        return FALSE;
    } else {
        if (term_child_count(term) > 0) {
            for (uint8_t i = 0; i < term_child_count(term); i++) {
                if (eval_done(term_get_child(term, i)) == FALSE) {
                    return FALSE;
                }
            }
            return TRUE;
        } else {
            return TRUE;
        }
    }
}