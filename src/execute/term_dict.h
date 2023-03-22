#ifndef _TERM_DICT_H
#define _TERM_DICT_H

#include <string.h>
#include <stdint.h>

#include "execute\term.h"

#include "global.h"
#include "config.h"

#include "memory.h"

struct _TermDict {
    size_t count;
    size_t capacity; // The number of elements it can hold without expanding
    struct Term* terms;
};
typedef struct _TermDict* TermDict_t;

TermDict_t  make_empty_term_dict    ();
enum ErrorCode get_term_index          (TermDict_t, struct Term, INDEX*);
enum ErrorCode add_term_to_dict        (TermDict_t, struct Term, size_t*);
enum ErrorCode strip_term_dict         (TermDict_t);
struct Term get_term_by_id          (TermDict_t, INDEX);
void        free_term_dict          (TermDict_t*);

// Adds more memory to the dict.
enum ErrorCode   _grow_term_dict       (TermDict_t);

#endif