#ifndef _PROGRAM_H_
#define _PROGRAM_H_

// #define VALUE_TYPE_DELTA        0
// #define VALUE_TYPE_REFERENCE    1
// #define VALUE_TYPE_STRING       2
// #define VALUE_TYPE_RATIONAL     3
// #define VALUE_TYPE_PRIMOP       4

#define PROGRAM_TYPE_LEAF       0
#define PROGRAM_TYPE_STEM       1
#define PROGRAM_TYPE_FORK       2
// #define PROGRAM_TYPE_VALUE      3

#include <stdint.h>

#include "global.h"
#include "rational/rational.h"
#include "serialize/serialize.h"

struct ProgramPair {
    struct Program* prg0;
    struct Program* prg1;
};

// struct Program* program_make
//     (struct Value*, struct Program*, struct Program*);
struct Program*     program_with_label  (char*, struct Program*);
struct Program*     program_make_leaf   ();
struct Program*     program_make_stem   (struct Program*);
struct Program*     program_make_fork   (struct Program*,struct Program*);
void                program_set_label   (struct Program*, char*);
char*               program_get_label   (struct Program*);
uint8_t             program_get_type    (struct Program*);
struct Program*     program_get_child   (struct Program*, uint8_t);
void                program_free        (struct Program*);
struct Program*     program_copy        (struct Program*);
BOOL                program_apply       (struct Program*, struct Program*);
size_t              program_get_size    (struct Program*);
struct ProgramPair  program_extract_subprograms (struct Program*);

void                program_print       (struct Program*);
void                program_serialize   (Serializer_t*, struct Program*);
struct Program*     program_deserialize (Serializer_t*);

#endif