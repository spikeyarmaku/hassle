#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>
#include <stdio.h>

#include "global.h"

struct Tree*    tree_make           (uint8_t);
struct Tree*    tree_copy           (struct Tree*);
void            tree_free           (struct Tree*);
void            tree_free_toplevel  (struct Tree*);
uint8_t         tree_child_count    (struct Tree*);
struct Tree*    tree_get_child      (struct Tree*, uint8_t);
struct Tree*    tree_apply          (struct Tree*, struct Tree*);
uint8_t*        tree_serialize      (struct Tree*, size_t*);
struct Tree*    tree_deserialize    (uint8_t*);

void            tree_print          (struct Tree*);

#endif