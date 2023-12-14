#ifndef _CODE_H_
#define _CODE_H_

#include "global.h"

struct Code code_make       (uint8_t*);
uint8_t     code_read_byte  (struct Code*);
VM_WORD     code_read_word  (struct Code*, uint8_t);

#endif