#ifndef _FRAME_H_
#define _FRAME_H_

#include "closure.h"

typedef struct Frame Frame_t;

Frame_t*    frame_make  (char*, Closure_t*, Frame_t*);
void        frame_free  (Frame_t*);

#endif