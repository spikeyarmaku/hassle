#ifndef _POOL_H_
#define _POOL_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "global.h"

struct PtrPool*     ptrpool_make        ();
struct PtrPool*     ptrpool_make_fixed  (size_t);
void                ptrpool_add         (struct PtrPool*, void*);
void                ptrpool_add_unsafe  (struct PtrPool*, void*);
size_t              ptrpool_get_count   (struct PtrPool*);
void                ptrpool_set         (struct PtrPool*, size_t, void*);
void*               ptrpool_get         (struct PtrPool*, size_t);

struct BytePool*    bytepool_make       (size_t);
void                bytepool_add_elems  (struct BytePool*, size_t, uint8_t*);
void                bytepool_add_elem   (struct BytePool*, uint8_t*);
uint8_t             bytepool_get_byte   (struct BytePool*, size_t);

#endif