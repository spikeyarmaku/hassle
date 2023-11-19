#ifndef _TREE_H_
#define _TREE_H_

#define TREE_TYPE_PROGRAM   0
#define TREE_TYPE_APPLY     1

#include <stdint.h>

#include "program.h"

struct Tree*    tree_make_program   (struct Program*);
struct Tree*    tree_make_apply     (struct Tree*, struct Tree*);
struct Tree*    tree_apply          (struct Tree*, struct Tree*);
struct Tree*    tree_apply_programs (struct Program*, struct Program*);
uint8_t         tree_get_type       (struct Tree*);
struct Program* tree_get_program    (struct Tree*);
struct Tree*    tree_get_apply      (struct Tree*, uint8_t);
struct Tree*    tree_copy           (struct Tree*);
void            tree_free           (struct Tree*);
void            tree_serialize      (Serializer_t*, struct Tree*);
struct Tree*    tree_deserialize    (Serializer_t*);
void            tree_print          (struct Tree*);
size_t          tree_get_size       (struct Tree*);

// Stuff usd in nStar
BOOL            tree_is_reference   (struct Tree*);
char*           tree_get_reference  (struct Tree*);
struct Tree*    tree_extract        (struct Tree*);

#endif