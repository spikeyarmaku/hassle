#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h> // DEBUG
#include <string.h>

#include "rational\alnat.h"

/*
Rational_t usage:

Construction:
  A Rational_t can be made from a base-10 string representation using
  `string_to_rational`.

Destruction:
  Call `rational_free`.
*/

struct _Rational {
    int8_t sign;
    Alnat_t numerator;
    Alnat_t denominator; // If it is NULL, treat it as 1
};

typedef struct _Rational* Rational_t;

Rational_t      _rational_make          ();
Rational_t      string_to_rational      (char*);
void            rational_free           (Rational_t);
BOOL            rational_is_valid       (Rational_t);
BOOL            rational_is_equal       (Rational_t, Rational_t);
void            rational_simplify       (Rational_t);
void            rational_reciprocate    (Rational_t);
char*           rational_to_string      (Rational_t);
void            rational_print         (Rational_t);

Rational_t      rational_add            (Rational_t, Rational_t);
Rational_t      rational_sub            (Rational_t, Rational_t);
Rational_t      rational_mul            (Rational_t, Rational_t);
Rational_t      rational_div            (Rational_t, Rational_t);

#endif