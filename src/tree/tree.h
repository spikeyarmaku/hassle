#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>

#include "global.h"

struct Tree*    tree_make           ();
void            tree_free           (struct Tree*);
void            tree_free_toplevel  (struct Tree*);
size_t          tree_child_count    (struct Tree*);
struct Tree*    tree_get_child      (struct Tree*, size_t);
struct Tree*    tree_apply          (struct Tree*, struct Tree*);
uint8_t*        tree_serialize      (struct Tree*, size_t*);
struct Tree*    tree_deserialize    (uint8_t*);

#endif