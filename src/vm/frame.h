#ifndef _FRAME_H_
#define _FRAME_H_

#include "closure.h"

typedef struct Frame Frame_t;

Frame_t*    frame_make      (char*, Closure_t*, Frame_t*);
void        frame_update    (Frame_t*, Closure_t*);
Closure_t*  frame_lookup    (Frame_t*, char*);
void        frame_free      (Frame_t*);

// DEBUG
char*       frame_get_name      (Frame_t*); // DEBUG
Closure_t*  frame_get_value     (Frame_t*); // DEBUG
Frame_t*    frame_get_parent    (Frame_t*); // DEBUG

#endif