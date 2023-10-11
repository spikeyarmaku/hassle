#include "term.h"

struct Term {
    uint8_t type;
    union {
        struct {
            struct Term* term_left;
            struct Term* term_right;
        };
        char* str_val;
        Rational_t* rat_val;
    };
};

struct Term* term_make_node() {
    struct Term* term = allocate_mem("term_make", NULL,
        sizeof(struct Term));
    term->type = TERM_TYPE_LEAF;
    term->term_left = NULL;
    term->term_right = NULL;
    return term;
}

struct Term* term_make_sym(char* str) {
    struct Term* term = term_make_node();
    term->type = TERM_TYPE_SYMBOL;
    term->str_val = str;
    return term;
}

struct Term* term_make_str(char* str) {
    struct Term* term = term_make_node();
    term->type = TERM_TYPE_STRING;
    term->str_val = str;
    return term;
}

struct Term* term_make_rat(Rational_t* rat) {
    struct Term* term = term_make_node();
    term->type = TERM_TYPE_RATIONAL;
    term->rat_val = rat;
    return term;
}

struct Term* term_apply(struct Term* term1, struct Term* term2) {
    if (term1->type == TERM_TYPE_LEAF || term1->type == TERM_TYPE_FORK) {
        struct Term* term = term_make_node();
        term->type = TERM_TYPE_FORK;
        term->term_left = term1;
        term->term_right = term2;
        return term;
    } else {
        printf("term_apply: Trying to apply to a value\n"); exit(1);
    }
}

void term_set_children(struct Term* term, struct Term* child_left,
    struct Term* child_right)
{
    term->term_left = child_left;
    term->term_right = child_right;
}

struct Term* term_copy(struct Term* term) {
    if (term->type == TERM_TYPE_FORK) {
        return term_apply(term_copy(term->term_left),
            term_copy(term->term_right));
    } else {
        switch (term->type) {
            case TERM_TYPE_LEAF: {
                return term_make_node();
            }
            case TERM_TYPE_SYMBOL: {
                return term_make_sym(str_cpy(term->str_val));
            }
            case TERM_TYPE_STRING: {
                return term_make_str(str_cpy(term->str_val));
            }
            case TERM_TYPE_RATIONAL: {
                return term_make_rat(rational_copy(term->rat_val));
            }
            default: {
                return NULL;
            }
        }
    }
}

void term_free_toplevel(struct Term* term) {
    free_mem("term_free", term);
}

void term_free(struct Term* term) {
    switch (term->type) {
        case TERM_TYPE_LEAF: {
            break;
        }
        case TERM_TYPE_FORK: {
            term_free(term->term_left);
            term_free(term->term_right);
            break;
        }
        case TERM_TYPE_RATIONAL: {
            free_mem("term_free/rat", term->rat_val);
            break;
        }
        case TERM_TYPE_STRING: {
            free_mem("term_free/str", term->str_val);
            break;
        }
        case TERM_TYPE_SYMBOL: {
            free_mem("term_free/str", term->str_val);
            break;
        }
        default: {
            break;
        }
    }
    term_free_toplevel(term);
}

void term_serialize(Serializer_t* serializer, struct Term* term) {
    serializer_write(serializer, term->type);
    if (term->type == TERM_TYPE_FORK) {
        term_serialize(serializer, term->term_left);
        term_serialize(serializer, term->term_right);
    } else {
        // Primval
        switch(term->type) {
            case TERM_TYPE_LEAF: {
                break;
            }
            case TERM_TYPE_SYMBOL: {
                serializer_write_string(serializer, term->str_val);
                break;
            }
            case TERM_TYPE_STRING: {
                serializer_write_string(serializer, term->str_val);
                break;
            }
            case TERM_TYPE_RATIONAL: {
                rational_serialize(serializer, term->rat_val);
                break;
            }
        }
    }
}

// TODO
struct Term* term_deserialize(Serializer_t* serializer) {
    uint8_t type = serializer_read(serializer);
    switch (type) {
        case TERM_TYPE_FORK: {
            struct Term* term1 = term_deserialize(serializer);
            struct Term* term2 = term_deserialize(serializer);
            return term_apply(term1, term2);
        }
        case TERM_TYPE_LEAF: {
            return term_make_node();
        }
        case TERM_TYPE_SYMBOL: {
            return term_make_sym(serializer_read_string(serializer));
        }
        case TERM_TYPE_STRING: {
            return term_make_str(serializer_read_string(serializer));
        }
        case TERM_TYPE_RATIONAL: {
            return term_make_rat(rational_deserialize(serializer));
        }
        default: {
            return NULL;
        }
    }
}

BOOL term_is_value(struct Term* term) {
    return (term->type == TERM_TYPE_LEAF || term->type == TERM_TYPE_FORK) ?
        FALSE : TRUE;
}

BOOL term_is_symbol(char* symbol, struct Term* term) {
    return
        (term->type == TERM_TYPE_SYMBOL &&
        strcmp(symbol, term_get_sym(term)) == 0) ? TRUE : FALSE;
}

uint8_t term_type(struct Term* term) {
    return term->type;
}

char* term_get_sym(struct Term* term) {
    return term->str_val;
}

char* term_get_str(struct Term* term) {
    return term->str_val;
}

Rational_t* term_get_rat(struct Term* term) {
    return term->rat_val;
}

struct Term* term_child_left(struct Term* term) {
    return term->term_left;
}

struct Term* term_child_right(struct Term* term) {
    return term->term_right;
}

uint8_t term_app_level(struct Term* term) {
    if (term->type == TERM_TYPE_FORK) {
        return 1 + term_app_level(term->term_left);
    } else {
        return 0;
    }
}

// Traverses the tree in one direction for the amount specified
// E.g. traverse(tree, 3) = tree->right->right->right
// traverse(tree, -2) = tree->left->left
struct Term* term_traverse(struct Term* term, int8_t dir) {
    if (dir == 0) {
        return term;
    }
    return (dir > 0) ?
        term_traverse(term->term_right, dir - 1) :
        term_traverse(term->term_left, dir + 1);
}

void term_print(struct Term* term) {
    if (term->type != TERM_TYPE_FORK) {
        printf("[");
        switch (term->type) {
            case TERM_TYPE_LEAF: {
                printf("Δ");
                break;
            }
            case TERM_TYPE_RATIONAL: {
                rational_print(term->rat_val);
                break;
            }
            case TERM_TYPE_STRING: {
                printf("\"%s\"", term->str_val);
                break;
            }
            case TERM_TYPE_SYMBOL: {
                printf("%s", term->str_val);
                break;
            }
        }
        printf("]");
    } else {
        printf("(");
        term_print(term->term_left);
        printf(" ");
        term_print(term->term_right);
        printf(")");
    }
}
