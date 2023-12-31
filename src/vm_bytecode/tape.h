#ifndef _TAPE_H_
#define _TAPE_H_

#include "global.h"
#include "memory.h"

#include "code.h"

struct Tape {
    size_t pc;
    uint8_t* code;
};

struct Tape*    tape_make       ();
void            tape_set        (struct Tape*, const uint8_t*);
uint8_t         tape_read_byte  (struct Tape*);
VM_WORD         tape_read_word  (struct Tape*, uint8_t);
void            tape_free       (struct Tape*);

void            tape_print      (struct Tape*);

#endif