#ifndef _FRAME_H_
#define _FRAME_H_

#include "global.h"

#include "closure.h"
#include "serialize\serialize.h"

typedef struct Frame Frame_t;
typedef struct Heap Heap_t;

Frame_t*    frame_make          (char*, Closure_t*, Frame_t*);
void        frame_update        (Frame_t*, Closure_t*);
Closure_t*  frame_lookup        (Frame_t*, char*, OUT Frame_t**);
void        frame_serialize     (Serializer_t*, Heap_t*, Frame_t*);
Frame_t*    frame_deserialize   (Serializer_t*, Heap_t*);
// void        frame_free          (Frame_t*);

// DEBUG
char*       frame_get_name      (Frame_t*); // DEBUG
Closure_t*  frame_get_value     (Frame_t*); // DEBUG
Frame_t*    frame_get_parent    (Frame_t*); // DEBUG

#endif