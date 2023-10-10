#ifndef _TERM_H_
#define _TERM_H_

#define TERM_TYPE_FORK      0
#define TERM_TYPE_LEAF      1
#define TERM_TYPE_SYMBOL    2
#define TERM_TYPE_STRING    3
#define TERM_TYPE_RATIONAL  4

#include <stdint.h>
#include <stdio.h>

#include "global.h"
#include "serialize/serialize.h"
#include "rational/rational.h"

struct Term*    term_make_node      ();
struct Term*    term_make_sym       (char*);
struct Term*    term_make_str       (char*);
struct Term*    term_make_rat       (Rational_t*);
struct Term*    term_apply          (struct Term*, struct Term*);
struct Term*    term_copy           (struct Term*);
void            term_free           (struct Term*);
void            term_free_toplevel  (struct Term*);
void            term_serialize      (Serializer_t*, struct Term*);
struct Term*    term_deserialize    (Serializer_t*);
BOOL            term_is_value       (struct Term*);
BOOL            term_is_symbol      (char*, struct Term*);
uint8_t         term_type           (struct Term*);
char*           term_get_sym        (struct Term*);
char*           term_get_str        (struct Term*);
Rational_t*     term_get_rat        (struct Term*);
struct Term*    term_child_left     (struct Term*);
struct Term*    term_child_right    (struct Term*);
uint8_t         term_app_level      (struct Term*);
struct Term*    term_traverse       (struct Term*, int8_t);

void            term_print          (struct Term*);

#endif