#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "term.h"

typedef struct VM VM_t;

struct VMData {
    uint8_t* data;
    size_t data_size;
};

enum EvalState {EvalFinished, EvalRunning};

VM_t*           vm_init                 (Expr_t*);
void            vm_set_control_to_expr  (VM_t*, Expr_t*);
enum EvalState  vm_step                 (VM_t*);
Term_t*         vm_run                  (VM_t*);
struct VMData   vm_serialize            (VM_t*, uint8_t);
VM_t*           vm_deserialize          (uint8_t*);
void            vm_free                 (VM_t*);

#endif