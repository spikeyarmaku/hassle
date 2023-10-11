#ifndef _VM_H_
#define _VM_H_

#include "global.h"

#include "tree/term.h"

struct VMData {
    uint8_t* data;
    size_t data_size;
};

enum EvalState {EvalFinished, EvalRunning};

struct VM*      vm_init         ();
void            vm_set_term     (struct VM*, struct Term*);
enum EvalState  vm_step         (struct VM*);
struct Term*    vm_run          (struct VM*);
struct VMData   vm_serialize    (struct VM*, uint8_t);
void            vm_reset        (struct VM*);
void            vm_free         (struct VM*);

#endif