#include "program.h"

struct Program {
    char* label;
    struct Program* children[2];
};

struct Program* program_make(char* label, struct Program* left,
    struct Program* right)
{
    struct Program* program = allocate_mem("program_make", NULL,
        sizeof(struct Program));
    if (label != NULL) {
        program_set_label(program, label);
    } else {
        program->label = NULL;
    }
        program->children[0] = left;
        program->children[1] = right;
    return program;
}

struct Program* program_with_label(char* label, struct Program* program) {
    program_set_label(program, label);
    return program;
}

struct Program* program_make_leaf() {
    return program_make(NULL, NULL, NULL);
}

struct Program* program_make_stem(struct Program* prg0) {
    return program_make(NULL, prg0, NULL);
}

struct Program* program_make_fork(struct Program* prg0,struct Program* prg1) {
    return program_make(NULL, prg0, prg1);
}

void program_set_label(struct Program* program, char* label) {
    program->label = str_cpy(label);
}

char* program_get_label(struct Program* program) {
    return program->label;
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

struct Program* program_get_child(struct Program* program, uint8_t index) {
        return program->children[index];
}

void program_free(struct Program* program) {
    if (program == NULL) {
        return;
    }
    free_mem("program_free/label", program->label);
    program_free(program->children[0]);
    program_free(program->children[1]);
    free_mem("program_free", program);
}

struct Program* program_copy(struct Program* program) {
    if (program == NULL) {
        return NULL;
    }

    return
        program_make(program->label == NULL ? NULL : str_cpy(program->label),
            program_copy(program->children[0]),
            program_copy(program->children[1]));
}

// Try to add prg1 as a child of prg0, return TRUE if successful (prg0 is leaf
// or stem), FALSE otherwise
BOOL program_apply(struct Program* prg0, struct Program* prg1) {
    switch (program_get_type(prg0)) {
        case PROGRAM_TYPE_LEAF: {
            prg0->children[0] = prg1;
            return TRUE;
        }
        case PROGRAM_TYPE_STEM: {
            prg0->children[1] = prg1;
            return TRUE;
        }
        case PROGRAM_TYPE_FORK: {
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

    return 1 + program_get_size(program->children[0]) +
        program_get_size(program->children[1]);
}

void program_print(struct Program* program) {
    uint8_t type = program_get_type(program);
    if (type > 0) {
        printf("(");
    }
    // Label or Δ
    if (program->label != NULL) {
        printf("%s", program->label);
    } else {
        printf("Δ");
    }
    // Children
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

    serializer_write_string(serializer, program->label);
    serializer_write(serializer, program_get_type(program));
    program_serialize(serializer, program->children[0]);
    program_serialize(serializer, program->children[1]);
}

struct Program* program_deserialize(Serializer_t* serializer) {
    // TODO
    return NULL;
}
