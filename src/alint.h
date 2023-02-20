#ifndef _ALINT_H_
#define _ALINT_H_

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h> // DEBUG

// Change these together. DIVIDER shall always be one greater than the biggest
// number that can be stored in ALINT_TYPE
#define ALINT_TYPE uint8_t
// Important: if the << operator is used, it must be put in parenthesis
#define ALINT_MAX (1<<8)

// Arbitrary Length INTeger, least significant byte first
// TODO Think about a more memory-efficient representation
// The current way of storing an alint is very wasteful, as each byte comes with
// a pointer's worth of overhead
struct Alint {
    ALINT_TYPE num;
    struct Alint* next;
};

struct Alint* string_to_alint(char*);
void debug_print_alint(struct Alint*);
char* alint_to_string(struct Alint*);
void destroy_alint(struct Alint*);
struct Alint* make_null_alint();
uint8_t is_null_alint(struct Alint*);

struct Alint* add_alint(struct Alint*, struct Alint*);
struct Alint* make_complement_alint(struct Alint*);
struct Alint* sub_alint(struct Alint*, struct Alint*);
void strip_alint(struct Alint*);
int8_t compare_alint(struct Alint*, struct Alint*);

void gcd(struct Alint*, struct Alint*);

#endif