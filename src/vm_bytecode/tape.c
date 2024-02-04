#include "tape.h"

#include <stdio.h>

struct Tape* tape_make() {
    struct Tape* tape = allocate_mem("tape_make", NULL, sizeof(struct Tape));
    tape->pc = 0;
    tape->code = NULL;
    return tape;
}

void tape_set(struct Tape* tape, const uint8_t* code) {
    // casting away const, probably bad design; TODO fix it
    tape->code = (uint8_t*)code;
    tape->pc = 0;
}

uint8_t tape_read_byte(struct Tape* tape) {
    uint8_t result = tape->code[tape->pc];
    tape->pc++;
    return result;
}

VM_WORD tape_read_word(struct Tape* tape, uint8_t word_size) {
    VM_WORD result = bytes_to_word(tape->code + tape->pc, word_size);
    tape->pc += word_size;
    return result;
}

void tape_free(struct Tape* tape) {
    free_mem("tape_free/code", tape->code);
    free_mem("tape_free", tape);
}

void tape_print(struct Tape* tape) {
    for (size_t i = 0; tape->code[i] != OP_RETURN;) {
        printf("%llu: ", i);
        switch (tape->code[i]) {
            case OP_MKAGENT: {
                printf("MKAGENT r%d %s\n",
                    tape->code[i + 1], AgentNameTable[tape->code[i + 2]]);
                i += 3;
                break;
            }
            case OP_MKNAME: {
                printf("MKNAME r%d\n",
                    tape->code[i + 1]);
                i += 2;
                break;
            }
            case OP_CONNECT: {
                printf("CONNECT r%d->p%d r%d\n",
                    tape->code[i + 1], tape->code[i + 2], tape->code[i + 3]);
                i += 4;
                break;
            }
            case OP_PUSH: {
                printf("PUSH r%d r%d\n",
                    tape->code[i + 1], tape->code[i + 2]);
                i += 3;
                break;
            }
            case OP_STORE: {
                printf("STORE r%d g%llu\n", tape->code[i + 1],
                    bytes_to_word(tape->code + i + 2, sizeof(size_t)));
                i += 10;
                break;
            }
            case OP_LOAD: {
                printf("LOAD g%llu r%d\n",
                    bytes_to_word(tape->code + i + 1, sizeof(size_t)),
                    tape->code[i + 1 + sizeof(size_t)]);
                i += 10;
                break;
            }
            case OP_RETURN: {
                printf("RETURN\n");
                break;
            }
        }
    }
}
