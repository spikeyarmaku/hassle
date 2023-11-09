#ifndef _VM_H_
#define _VM_H_

#include "global.h"

#include "tree.h"
#include "program.h"

struct VMData {
    uint8_t* data;
    size_t data_size;
};

enum EvalState {EvalFinished, EvalRunning};

struct VM*      vm_make         ();
void            vm_populate     (struct VM*, struct Tree*);
enum EvalState  vm_step         (struct VM*);
struct Program* vm_run          (struct VM*);
struct VMData   vm_serialize    (struct VM*, uint8_t);
struct VM*      vm_deserialize  (uint8_t*);
void            vm_reset        (struct VM*);
void            vm_free         (struct VM*);

#endif