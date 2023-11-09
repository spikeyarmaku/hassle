#ifndef _KIN_H_
#define _KIN_H_

#include "global.h"
#include "tree/tree.h"
#include "serialize/serialize.h"

struct Kin*     kin_make        (BOOL, struct Tree*);
void            kin_free        (struct Kin*);
BOOL            kin_is_parent   (struct Kin*);
struct Tree*    kin_get_tree    (struct Kin*);
void            kin_serialize   (Serializer_t*, struct Kin*);
struct Kin*     kin_deserialize (Serializer_t*);

#endif