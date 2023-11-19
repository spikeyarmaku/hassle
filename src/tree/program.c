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
    uint8_t type;
    union {
        struct Value* value;
        struct Program* children[2];
    };
};

struct Value* value_make_ref(char* ref) {
    struct Value* value =
        allocate_mem("_value_make", NULL, sizeof(struct Value));
    value->type = VALUE_TYPE_REFERENCE;
    value->str_val = str_cpy(ref);
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

char* value_get_ref(struct Value* value) {
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
        case VALUE_TYPE_REFERENCE: {
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
        case VALUE_TYPE_REFERENCE: {
            return value_make_ref(str_cpy(value->str_val));
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
            case VALUE_TYPE_REFERENCE: {
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
        case VALUE_TYPE_REFERENCE: {
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

struct Program* program_make(uint8_t type, struct Value* value,
    struct Program* left, struct Program* right)
{
    struct Program* program = allocate_mem("program_make", NULL,
        sizeof(struct Program));
    program->type = type;
    if (type == PROGRAM_TYPE_VALUE) {
        program->value = value;
    } else {
        program->children[0] = left;
        program->children[1] = right;
    }
    return program;
}

struct Program* program_make_leaf() {
    return program_make(PROGRAM_TYPE_LEAF, NULL, NULL, NULL);
}

struct Program* program_make_stem(struct Program* prg0) {
    return program_make(PROGRAM_TYPE_STEM, NULL, prg0, NULL);
}

struct Program* program_make_fork(struct Program* prg0,struct Program* prg1) {
    return program_make(PROGRAM_TYPE_FORK, NULL, prg0, prg1);
}

struct Program* program_make_value(struct Value* val) {
    return program_make(PROGRAM_TYPE_VALUE, val, NULL, NULL);
}

uint8_t program_get_type(struct Program* program) {
    return program->type;
}

struct Value* program_get_value(struct Program* program) {
    if (program->type == PROGRAM_TYPE_VALUE) {
        return program->value;
    } else {
        return NULL;
    }
}

struct Program* program_get_child(struct Program* program, uint8_t index) {
    if (index < program->type && program->type != PROGRAM_TYPE_VALUE) {
        return program->children[index];
    } else {
        return NULL;
    }
}

void program_free(struct Program* program) {
    if (program == NULL) {
        return;
    }
    if (program->type == PROGRAM_TYPE_VALUE) {
        value_free(program->value);
    } else {
        program_free(program->children[0]);
        program_free(program->children[1]);
    }
    free_mem("program_free", program);
}

struct Program* program_copy(struct Program* program) {
    if (program == NULL) {
        return NULL;
    }

    if (program->type == PROGRAM_TYPE_VALUE) {
        return
            program_make(program->type, value_copy(program->value), NULL, NULL);
    } else {
        return
            program_make(program->type,
                NULL,
                program_copy(program->children[0]),
                program_copy(program->children[1]));
    }
}

// Try to add prg1 as a child of prg0, return TRUE if successful (prg0 is leaf
// or stem), FALSE otherwise
BOOL program_apply(struct Program* prg0, struct Program* prg1) {
    switch (program_get_type(prg0)) {
        case PROGRAM_TYPE_LEAF: {
            prg0->children[0] = prg1;
            prg0->type = PROGRAM_TYPE_STEM;
            return TRUE;
        }
        case PROGRAM_TYPE_STEM: {
            prg0->children[1] = prg1;
            prg0->type = PROGRAM_TYPE_FORK;
            return TRUE;
        }
        case PROGRAM_TYPE_FORK: {
            return FALSE;
        }
        case PROGRAM_TYPE_VALUE: {
            return FALSE;
        }
        default: {
            fatal("program_apply: invalid program type %d\n",
                program_get_type(prg0));
            return FALSE;
        }
    }
}

size_t program_get_size(struct Program* program) {
    if (program == NULL) {
        return 0;
    }
    
    if (program->type == PROGRAM_TYPE_VALUE) {
        return 1;
    }

    return 1 + program_get_size(program->children[0]) +
        program_get_size(program->children[1]);
}

void program_print(struct Program* program) {
    uint8_t type = program_get_type(program);
    if (type > 0) {
        printf("(");
    }
    if (program->type == PROGRAM_TYPE_VALUE) {
        value_print(program->value);
    } else {
        if (type == 0) {
            printf("Δ");
        } else {
            for (uint8_t i = 0; i < type; i++) {
                program_print(program_get_child(program, i));
            }
        }
    }
    if (type > 0) {
        printf(")");
    }
}

void program_serialize(Serializer_t* serializer, struct Program* program) {
    if (program == NULL) {
        return;
    }

    serializer_write(serializer, program->type);
    if (program->type == PROGRAM_TYPE_VALUE) {
        value_serialize(serializer, program_get_value(program));
    } else {
        program_serialize(serializer, program->children[0]);
        program_serialize(serializer, program->children[1]);
    }
}

struct Program* program_deserialize(Serializer_t* serializer) {
    // TODO
    return NULL;
}
