/*
MSD - Most Significatn Digit
LSD - Least Significant Digit
*/

#ifndef _ALNAT_H_
#define _ALNAT_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "global.h"

#include "memory.h"

// Arbitrary Length NATural, least significant digit first
// Each byte contains a digit in base 128, and each byte has its highest bit set
// to one, except for the last byte, which has its highest bit set to 0.
typedef uint8_t* Alnat_t;

struct AlnatDiv {
    Alnat_t quot;
    Alnat_t rem;
};
typedef struct AlnatDiv AlnatDiv_t;

Alnat_t             string_to_alnat             (char*);
char*               alnat_to_string             (Alnat_t);
void                alnat_free                  (Alnat_t);
Alnat_t             alnat_make_single_digit     (uint8_t);
BOOL                alnat_is_null               (Alnat_t);
BOOL                alnat_is_equal              (Alnat_t, Alnat_t);

Alnat_t             alnat_add                   (Alnat_t, Alnat_t);
Alnat_t             alnat_sub                   (Alnat_t, Alnat_t, int8_t*);
Alnat_t             alnat_mul                   (Alnat_t, Alnat_t);
AlnatDiv_t          alnat_div                   (Alnat_t, Alnat_t);

Alnat_t             alnat_gcd                   (Alnat_t, Alnat_t);

void                alnat_print                 (Alnat_t);
void                alnat_print_bytes           (Alnat_t);
void                alnat_print_raw_bytes       (Alnat_t);

Alnat_t             alnat_copy                  (Alnat_t);

#endif