#include "tape.h"

struct Tape* tape_make() {
    struct Tape* tape = allocate_mem("tape_make", NULL, sizeof(struct Tape));
    tape->pc = 0;
    tape->code = NULL;
    return tape;
}

void tape_set(struct Tape* tape, uint8_t* code) {
    tape->code = code;
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
