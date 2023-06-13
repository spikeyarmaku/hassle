#ifndef _CLOSURE_H_
#define _CLOSURE_H_

// #include "frame.h"
#include "term.h"

typedef struct Frame Frame_t;

typedef struct Closure Closure_t;

Closure_t*  closure_make        (Term_t*, Frame_t*);
Term_t*     closure_get_term    (Closure_t*);
Frame_t*    closure_get_frame   (Closure_t*);
void        closure_free        (Closure_t*);

#endif