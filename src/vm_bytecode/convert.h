#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "global.h"

#include "tree/tree.h"
#include "tree/program.h"

#include "containers.h"

struct ByteArray* convert_from_tree(struct Tree*, uint8_t);

#endif