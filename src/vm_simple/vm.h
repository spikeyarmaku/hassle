#ifndef _VM_H_
#define _VM_H_

#include "global.h"

#include "vm_simple/agent.h"
#include "vm_simple/eq_stack.h"
#include "tree/tree.h"
#include "tree/program.h"

struct VM {
    struct EqStack* eq_stack;
    struct Agent* root_agent;
};

struct VMData {
    uint8_t* data;
    size_t data_size;
};

enum EvalState {EvalFinished, EvalRunning};

struct VM*      vm_make         ();
void            vm_populate     (struct VM*, struct Tree*);
void            vm_push_eq      (struct VM*, struct Agent*, struct Agent*);
struct VMData   vm_serialize    (struct VM*, uint8_t);
void            vm_free         (struct VM*);

#endif