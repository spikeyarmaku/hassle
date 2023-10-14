#ifndef _TERM_H_
#define _TERM_H_

#define TERM_TYPE_DELTA     0
#define TERM_TYPE_SYMBOL    1
#define TERM_TYPE_STRING    2
#define TERM_TYPE_RATIONAL  3
#define TERM_TYPE_PRIMOP    4

#include <stdint.h>
#include <stdio.h>

#include "global.h"
#include "serialize/serialize.h"
#include "rational/rational.h"

struct Term*    term_make_node      ();
struct Term*    term_make_sym       (char*);
struct Term*    term_make_str       (char*);
struct Term*    term_make_rat       (Rational_t*);
struct Term*    term_make_primop    (uint8_t);
struct Term*    term_apply          (struct Term*, struct Term*);
struct Term*    term_copy           (struct Term*);
void            term_free           (struct Term*);
void            term_free_node      (struct Term*);
void            term_serialize      (Serializer_t*, struct Term*);
struct Term*    term_deserialize    (Serializer_t*);
BOOL            term_is_symbol      (char*, struct Term*);
uint8_t         term_type           (struct Term*);
uint8_t         term_child_count    (struct Term*);
void            term_set_child      (struct Term*, uint8_t, struct Term*);
struct Term*    term_detach_last    (struct Term*);
char*           term_get_sym        (struct Term*);
char*           term_get_str        (struct Term*);
Rational_t*     term_get_rat        (struct Term*);
uint8_t         term_get_primop     (struct Term*);
struct Term*    term_get_child      (struct Term*, uint8_t);

void            term_print          (struct Term*);

#endif