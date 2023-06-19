#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "term.h"

typedef struct VM VM_t;

enum EvalState {EvalFinished, EvalRunning};

VM_t*           vm_init                 (Expr_t*);
void            vm_set_control_to_expr  (VM_t*, Expr_t*);
enum EvalState  vm_step                 (VM_t*);
Term_t*         vm_run                  (VM_t*);
uint8_t*        vm_save                 (VM_t*);
VM_t*           vm_load                 (uint8_t*);
void            vm_free                 (VM_t*);
void            vm_serialize            (VM_t*);

#endif