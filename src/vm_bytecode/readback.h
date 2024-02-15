#ifndef _READBACK_H_
#define _READBACK_H_

#include "vm.h"
#include "agent.h"
#include "tree/program.h"
#include "tree/tree.h"

struct Tree*    readback        (struct VM*);
struct VMData   vm_serialize    (struct VM*, uint8_t);

#endif