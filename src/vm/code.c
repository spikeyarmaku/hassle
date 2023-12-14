#include "code.h"

struct Code {
    VM_WORD pc; // program counter
    uint8_t* data;
};

struct Code code_make(uint8_t* code_ptr) {
    struct Code code;
    code.pc = 0;
    code.data = code_ptr;
    return code;
}

uint8_t code_read_byte(struct Code* code) {
    uint8_t byte = code->data[code->pc];
    code->pc++;
    return byte;
}

VM_WORD code_read_word(struct Code* code, uint8_t word_size) {
    VM_WORD word = bytes_to_word(code->data[code->pc], word_size);
    code->pc += word_size;
    return word;
}