#include "program.h"

struct Value {
    uint8_t type;
    union {
        char* str_val;
        Rational_t* rat_val;
        uint8_t primop;
    };
};

struct Program {
    struct Value* value;
    struct Program* children[2];
};

struct Value* value_make_sym(char* sym) {
    struct Value* value =
        allocate_mem("_value_make", NULL, sizeof(struct Value));
    value->type = VALUE_TYPE_SYMBOL;
    value->str_val = str_cpy(sym);
    return value;
}

struct Value* value_make_str(char* str) {
    struct Value* value =
        allocate_mem("_value_make", NULL, sizeof(struct Value));
    value->type = VALUE_TYPE_STRING;
    value->str_val = str_cpy(str);
    return value;
}

struct Value* value_make_rat(Rational_t* rat) {
    struct Value* value =
        allocate_mem("_value_make", NULL, sizeof(struct Value));
    value->type = VALUE_TYPE_RATIONAL;
    value->rat_val = rat;
    return value;
}

struct Value* value_make_primop(uint8_t primop) {
    struct Value* value =
        allocate_mem("_value_make", NULL, sizeof(struct Value));
    value->type = VALUE_TYPE_PRIMOP;
    value->primop = primop;
    return value;
}

uint8_t value_get_type(struct Value* value) {
    if (value == NULL) {
        return VALUE_TYPE_DELTA;
    }
    return value->type;
}

char* value_get_str(struct Value* value) {
    return value->str_val;
}

char* value_get_sym(struct Value* value) {
    return value->str_val;
}

Rational_t* value_get_rat(struct Value* value) {
    return value->rat_val;
}

uint8_t value_get_primop(struct Value* value) {
    return value->primop;
}

void value_free(struct Value* value) {
    if (value == NULL) {
        return;
    }
    switch (value->type) {
        case VALUE_TYPE_SYMBOL: {
            free_mem("value_free", value->str_val);
            break;
        }
        case VALUE_TYPE_STRING: {
            free_mem("value_free", value->str_val);
            break;
        }
        case VALUE_TYPE_RATIONAL: {
            rational_free(value->rat_val);
            break;
        }
        case VALUE_TYPE_PRIMOP: {
            break;
        }
        default: {
            fatal("value_free: impossible value type: %d\n", value->type);
        }
    }
}


struct Value* value_copy(struct Value* value) {
    if (value == NULL) {
        return NULL;
    }
    switch (value->type) {
        case VALUE_TYPE_SYMBOL: {
            return value_make_sym(str_cpy(value->str_val));
        }
        case VALUE_TYPE_STRING: {
            return value_make_str(str_cpy(value->str_val));
        }
        case VALUE_TYPE_RATIONAL: {
            return value_make_rat(rational_copy(value->rat_val));
        }
        case VALUE_TYPE_PRIMOP: {
            return value_make_primop(value->primop);
        }
        default: {
            fatal("value_copy: impossible value type: %d\n", value->type);
            return NULL;
        }
    }
}

void value_serialize(Serializer_t* serializer, struct Value* value) {
    if (value == NULL) {
        serializer_write(serializer, 0);
    } else {
        serializer_write(serializer, value->type);
        switch (value->type) {
            case VALUE_TYPE_SYMBOL: {
                serializer_write_string(serializer, value->str_val);
                break;
            }
            case VALUE_TYPE_STRING: {
                serializer_write_string(serializer, value->str_val);
                break;
            }
            case VALUE_TYPE_RATIONAL: {
                rational_serialize(serializer, value->rat_val);
                break;
            }
            case VALUE_TYPE_PRIMOP: {
                serializer_write(serializer, value->primop);
                break;
            }
            default: {
                fatal("value_serialize: impossible value type: %d\n",
                    value->type);
            }
        }
    }
}

void value_print(struct Value* value) {
    switch (value_get_type(value)) {
        case VALUE_TYPE_DELTA: {
            printf("Δ");
            break;
        }
        case VALUE_TYPE_SYMBOL: {
            printf("%s", value->str_val);
            break;
        }
        case VALUE_TYPE_STRING: {
            printf("\"%s\"", value->str_val);
            break;
        }
        case VALUE_TYPE_RATIONAL: {
            printf("<");
            rational_print(value->rat_val);
            printf(">");
            break;
        }
        case VALUE_TYPE_PRIMOP: {
            printf("<Op %d>", value->primop);
            break;
        }
    }
}

// -----------------------------------------------------------------------------

struct Program* program_make(struct Value* value, struct Program* left,
    struct Program* right)
{
    struct Program* program = allocate_mem("program_make", NULL,
        sizeof(struct Program));
    program->value = value;
    program->children[0] = left;
    program->children[1] = right;
    return program;
}

uint8_t program_get_type(struct Program* program) {
    if (program->children[1] != NULL) {
        return PROGRAM_TYPE_FORK;
    } else {
        if (program->children[0] != NULL) {
            return PROGRAM_TYPE_STEM;
        } else {
            return PROGRAM_TYPE_LEAF;
        }
    }
}

struct Value* program_get_value(struct Program* program) {
    return program->value;
}

struct Program* program_get_child(struct Program* program, uint8_t index) {
    assert(index < 2);
    return program->children[index];
}

void program_free(struct Program* program) {
    if (program == NULL) {
        return;
    }
    value_free(program->value);
    program_free(program->children[0]);
    program_free(program->children[1]);
    free_mem("program_free", program);
}

struct Program* program_copy(struct Program* program) {
    if (program == NULL) {
        return NULL;
    }

    return
        program_make(
            value_copy(program->value),
            program_copy(program->children[0]),
            program_copy(program->children[1]));
}

// Try to add prg2 as a child of prg1, return TRUE if successful (prg1 is leaf
// or stem), FALSE otherwise
BOOL program_apply(struct Program* prg1, struct Program* prg2) {
    switch (program_get_type(prg1)) {
        case PROGRAM_TYPE_LEAF: {
            prg1->children[0] = prg2;
            return TRUE;
        }
        case PROGRAM_TYPE_STEM: {
            prg1->children[1] = prg2;
            return TRUE;
        }
        case PROGRAM_TYPE_FORK: {
            return FALSE;
        }
        default: {
            fatal("program_apply: invalid program type %d\n",
                program_get_type(prg1));
            return FALSE;
        }
    }
}

size_t program_get_size(struct Program* program) {
    switch (program_get_type(program)) {
        case PROGRAM_TYPE_LEAF: {
            return 1;
        }
        case PROGRAM_TYPE_STEM: {
            return 1 + program_get_size(program_get_child(program, 0));
        }
        case PROGRAM_TYPE_FORK: {
            return 1 + program_get_size(program_get_child(program, 0)) +
                program_get_size(program_get_child(program, 1));
        }
        default: {
            fatal("program_get_size: invalid program type %d\n",
                program_get_type(program));
            return 0;
        }
    }
}

void program_print(struct Program* program) {
    uint8_t type = program_get_type(program);
    if (type > 0) {
        printf("(");
    }
    value_print(program->value);
    for (uint8_t i = 0; i < type; i++) {
        program_print(program_get_child(program, i));
    }
    if (type > 0) {
        printf(")");
    }
}

void program_serialize(Serializer_t* serializer, struct Program* program) {
    if (program == NULL) {
        return;
    }

    value_serialize(serializer, program_get_value(program));
    serializer_write(serializer, program_get_type(program));
    program_serialize(serializer, program->children[0]);
    program_serialize(serializer, program->children[1]);
}

struct Program* program_deserialize(Serializer_t* serializer) {
    // TODO
    return NULL;
}
