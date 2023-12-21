#ifndef _EVAL_H_
#define _EVAL_H_

#include "global.h"

#include "vm_simple/rules.h"
#include "vm_simple/vm.h"

void    eval        (struct VM*);
BOOL    eval_step   (struct VM*);

#endif