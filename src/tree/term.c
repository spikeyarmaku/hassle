#include "term.h"

struct Term {
    uint8_t type;
    union {
        char* str_val;
        Rational_t* rat_val;
        uint8_t primop;
    };
    uint8_t child_count;
    struct Term** children;
};

struct Term* term_make_node() {
    struct Term* term = allocate_mem("term_make", NULL,
        sizeof(struct Term));
    term->type = TERM_TYPE_DELTA;
    term->child_count = 0;
    term->children = NULL;
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

struct Term* term_make_primop(uint8_t primop) {
    struct Term* term = term_make_node();
    term->type = TERM_TYPE_PRIMOP;
    term->primop = primop;
    return term;
}

struct Term* term_apply(struct Term* term1, struct Term* term2) {
    term1->child_count++;
    term1->children = allocate_mem("term_apply", term1->children,
        sizeof(struct Term) * term1->child_count);
    term1->children[term1->child_count - 1] = term2;
    return term1;
}

struct Term* term_copy(struct Term* term) {
    assert(term != NULL);
    struct Term* result;
    switch (term->type) {
        case TERM_TYPE_DELTA: {
            result = term_make_node();
            break;
        }
        case TERM_TYPE_SYMBOL: {
            result = term_make_sym(str_cpy(term->str_val));
            break;
        }
        case TERM_TYPE_STRING: {
            result = term_make_str(str_cpy(term->str_val));
            break;
        }
        case TERM_TYPE_RATIONAL: {
            result = term_make_rat(rational_copy(term->rat_val));
            break;
        }
        case TERM_TYPE_PRIMOP: {
            result = term_make_primop(term->primop);
            break;
        }
        default: {
            break;
        }
    }
    result->child_count = term->child_count;
    if (term->child_count > 0) {
        result->children = allocate_mem("term_copy", NULL,
            sizeof(struct Term) * term->child_count);
        for (uint8_t i = 0; i < term->child_count; i++) {
            result->children[i] = term_copy(term->children[i]);
        }
    } else {
        result->children = NULL;
    }
    return result;
}

// Free the value, the children array, and the pointer itself
void term_free_node(struct Term* term) {
    switch (term->type) {
        case TERM_TYPE_DELTA: {
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
        case TERM_TYPE_PRIMOP: {
            break;
        }
        default: {
            break;
        }
    }
    free_mem("term_free/children", term->children);
    free_mem("term_free", term);
}

void term_free(struct Term* term) {
    for (uint8_t i = 0; i < term->child_count; i++) {
        term_free(term->children[i]);
    }
    term_free_node(term);
}

// Type, value, child count, children
void term_serialize(Serializer_t* serializer, struct Term* term) {
    serializer_write(serializer, term->type);
    switch(term->type) {
        case TERM_TYPE_DELTA: {
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
        case TERM_TYPE_PRIMOP: {
            serializer_write(serializer, term->primop);
        }
    }
    serializer_write(serializer, term->child_count);
    for (uint8_t i = 0; i < term->child_count; i++) {
        term_serialize(serializer, term->children[i]);
    }
}

struct Term* term_deserialize(Serializer_t* serializer) {
    // Read node value
    uint8_t type = serializer_read(serializer);
    struct Term* result;
    switch (type) {
        case TERM_TYPE_DELTA: {
            result = term_make_node();
        }
        case TERM_TYPE_SYMBOL: {
            result = term_make_sym(serializer_read_string(serializer));
        }
        case TERM_TYPE_STRING: {
            result = term_make_str(serializer_read_string(serializer));
        }
        case TERM_TYPE_RATIONAL: {
            result = term_make_rat(rational_deserialize(serializer));
        }
        case TERM_TYPE_PRIMOP: {
            result = term_make_primop(serializer_read(serializer));
        }
        default: {
            return NULL;
        }
    }

    // Read children
    result->child_count = serializer_read(serializer);
    if (result->child_count > 0) {
        result->children = allocate_mem("term_deserialize", NULL,
            sizeof(struct Term) * result->child_count);
        for (uint8_t i = 0; i < result->child_count; i++) {
            result->children[i] = term_deserialize(serializer);
        }
    } else {
        result->children = NULL;
    }

    return result;
}

BOOL term_is_symbol(char* symbol, struct Term* term) {
    return
        (term->type == TERM_TYPE_SYMBOL &&
        strcmp(symbol, term_get_sym(term)) == 0) ? TRUE : FALSE;
}

uint8_t term_type(struct Term* term) {
    return term->type;
}

uint8_t term_child_count(struct Term* term) {
    return term->child_count;
}

void term_set_child(struct Term* term, uint8_t index, struct Term* child) {
    term->children[index] = child;
}

struct Term* term_detach_last(struct Term* term) {
    assert(term->child_count > 0);
    struct Term* child = term->children[term->child_count - 1];
    term->child_count--;
    if (term->child_count == 0) {
        free_mem("term_detach_last", term->children);
        term->children = NULL;
    } else {
        term->children = allocate_mem("term_detach_last", term->children,
            sizeof(struct Term) * term->child_count);
    }
    return child;
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

uint8_t term_get_primop(struct Term* term) {
    return term->primop;
}

struct Term* term_get_child(struct Term* term, uint8_t index) {
    return term->children[index];
}

void _term_print_node(struct Term* term) {
    printf("[");
    switch (term->type) {
        case TERM_TYPE_DELTA: {
            printf("*");
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
        case TERM_TYPE_PRIMOP: {
            printf("OP %d", term->primop);
            break;
        }
        default: {
            break;
        }
    }
    printf("]");
}

void term_print(struct Term* term) {
    if (term->child_count > 0) {
        printf("(");
    }
    _term_print_node(term);
    if (term->child_count > 0) {
        for (uint8_t i = 0; i < term->child_count; i++) {
            term_print(term->children[i]);
        }
        printf(")");
    }
}
