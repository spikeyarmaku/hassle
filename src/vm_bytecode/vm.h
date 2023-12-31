#ifndef _VM_H_
#define _VM_H_

#include "tree/tree.h"

#include "global.h"
#include "agent.h"
#include "containers.h"
#include "tape.h"

struct VMData {
    uint8_t* data;
    size_t data_size;
};

enum EvalState {EvalFinished, EvalRunning};

struct VM*      vm_make         (uint8_t);
void            vm_set_code     (struct VM*, uint8_t*);
void            vm_from_tree    (struct VM*, struct Tree*);
void            vm_eval         (struct VM*);
enum EvalState  vm_step         (struct VM*);
struct Tree*    vm_readback     (struct VM*);
void            vm_free         (struct VM*);
void            vm_reset        (struct VM*);

void            vm_print        (struct VM*);

#endif