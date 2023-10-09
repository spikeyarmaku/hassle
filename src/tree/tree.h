#ifndef _TREE_H_
#define _TREE_H_

#define LEAF_TYPE_DELTA     0
#define LEAF_TYPE_SYMBOL    1
#define LEAF_TYPE_STRING    2
#define LEAF_TYPE_RATIONAL  3

#include <stdint.h>
#include <stdio.h>

#include "global.h"
#include "serialize/serialize.h"
#include "rational/rational.h"

struct Tree*    tree_make           (uint8_t);
struct Tree*    tree_make_sym       (char*);
struct Tree*    tree_make_str       (char*);
struct Tree*    tree_make_rat       (Rational_t*);
void            tree_set_child      (struct Tree*, uint8_t, struct Tree*);
struct Tree*    tree_copy           (struct Tree*);
void            tree_free           (struct Tree*);
void            tree_free_toplevel  (struct Tree*);
uint8_t         tree_child_count    (struct Tree*);
struct Tree*    tree_get_child      (struct Tree*, uint8_t);
struct Tree*    tree_apply          (struct Tree*, struct Tree*);
void            tree_serialize      (Serializer_t*, struct Tree*);
struct Tree*    tree_deserialize    (Serializer_t*);
BOOL            tree_is_value       (struct Tree*);
BOOL            tree_is_symbol      (char*, struct Tree*);
uint8_t         tree_leaf_type      (struct Tree*);
char*           tree_get_sym        (struct Tree*);
char*           tree_get_str        (struct Tree*);
Rational_t*     tree_get_rat        (struct Tree*);

void            tree_print          (struct Tree*);

#endif