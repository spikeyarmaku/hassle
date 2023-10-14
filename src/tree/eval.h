#ifndef _EVAL_H_
#define _EVAL_H_

#include <stdlib.h>
#include <stdio.h>

#include "tree/term.h"

struct Term*    eval_step           (struct Term*);
struct Term*    eval                (struct Term*);
BOOL            eval_is_evaluable   (struct Term*);
BOOL            eval_done           (struct Term*);

#endif