#ifndef _DICT_H_
#define _DICT_H_

#include <string.h>
#include <stdint.h>

#include "global.h"
#include "config.h"

#include "memory.h"

/*
Dict usage:

Construction:
  To construct a dict, first make an empty dict using `make_empty_dict`, then
  add names (symbols) to it. When all the names are added, finalize it, so it
  will not take up more space than necessary.

Destruction:
  Call `free_dict` - it will free up the dictionary, and sets its name count to
  zero.
*/

// A list of symbols, appearing in the list in the order of their ID, so the
// first element's ID is 0, the next one's ID is 1, etc.
struct Dict {
    size_t count;
    char** names;
};

struct Dict make_empty_dict ();
ErrorCode   add_name        (struct Dict*, size_t*, char*, size_t*);
ErrorCode   finalize_dict   (struct Dict*, size_t*);
void        free_dict       (struct Dict*);

// Adds more memory to the dict.
ErrorCode   _grow_dict      (struct Dict*, size_t*);

#endif