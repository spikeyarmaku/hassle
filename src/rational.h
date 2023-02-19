#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h> // DEBUG

// Arbitrary Length INTeger
// TODO Think about a more memory-efficient representation
// The current way of storing an alint is very wasteful, as each byte comes with
// a pointer's worth of overhead
struct Alint {
    uint8_t byte; // If its typew changes, `divider` must be changed too in
                  // the method `string_to_alint`
    struct Alint* next;
};

struct Alint* string_to_alint(char*);
void debug_print_alint(struct Alint*);
char* alint_to_string(struct Alint*);
void destroy_alint(struct Alint);

struct Rational {
    //
    struct Alint numerator;
    struct Alint denominator;
};

struct Rational string_to_rational(char*);
void destroy_rational(struct Rational);

#endif