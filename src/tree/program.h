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

// struct Value*   value_make_ref      (char*);
// struct Value*   value_make_str      (char*);
// struct Value*   value_make_rat      (Rational_t*);
// struct Value*   value_make_primop   (uint8_t);
// uint8_t         value_get_type      (struct Value*);
// char*           value_get_ref       (struct Value*);
// char*           value_get_str       (struct Value*);
// Rational_t*     value_get_rat       (struct Value*);
// uint8_t         value_get_primop    (struct Value*);
// void            value_free          (struct Value*);
// struct Value*   value_copy          (struct Value*);
// void            value_serialize     (Serializer_t*, struct Value*);
// void            value_print         (struct Value*);

// struct Program* program_make
//     (struct Value*, struct Program*, struct Program*);
struct Program* program_with_label  (char*, struct Program*);
struct Program* program_make_leaf   ();
struct Program* program_make_stem   (struct Program*);
struct Program* program_make_fork   (struct Program*,struct Program*);
void            program_set_label   (struct Program*, char*);
char*           program_get_label   (struct Program*);
// struct Program* program_make_value  (struct Value*);
uint8_t         program_get_type    (struct Program*);
// struct Value*   program_get_value   (struct Program*);
struct Program* program_get_child   (struct Program*, uint8_t);
void            program_free        (struct Program*);
struct Program* program_copy        (struct Program*);
BOOL            program_apply       (struct Program*, struct Program*);
size_t          program_get_size    (struct Program*);

void            program_print       (struct Program*);
void            program_serialize   (Serializer_t*, struct Program*);
struct Program* program_deserialize (Serializer_t*);

#endif