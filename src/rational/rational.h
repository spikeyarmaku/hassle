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
  Call `free_rational`.
*/

struct _Rational {
    int8_t sign;
    Alnat_t numerator;
    Alnat_t denominator; // If it is NULL, treat it as 1
};

typedef struct _Rational* Rational_t;

Rational_t      _make_rational          ();
Rational_t      string_to_rational      (char*);
void            free_rational           (Rational_t);
BOOL            is_valid_rational       (Rational_t);
BOOL            is_equal_rational       (Rational_t, Rational_t);
void            simplify                (Rational_t);
void            reciprocate             (Rational_t);
int             print_rational          (char*, Rational_t);
void            debug_print_rational    (Rational_t);

Rational_t      add_rational            (Rational_t, Rational_t);
Rational_t      sub_rational            (Rational_t, Rational_t);
Rational_t      mul_rational            (Rational_t, Rational_t);
Rational_t      div_rational            (Rational_t, Rational_t);

#endif