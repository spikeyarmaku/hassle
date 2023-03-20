#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h> // DEBUG
#include <string.h>

#include "rational\alnat.h"

/*
Rational usage:

Construction:
  A Rational can be made from a base-10 string representation using
  `string_to_rational`.

Destruction:
  Call `free_rational`.
*/

struct _Rational {
    int8_t sign;
    Alnat numerator;
    Alnat denominator; // If it is NULL, treat it as 1
};

typedef struct _Rational* Rational;

Rational    _make_rational          ();
Rational    string_to_rational      (char*);
void        free_rational           (Rational);
BOOL        is_valid_rational       (Rational);
void        simplify                (Rational);
void        reciprocate             (Rational);
void        debug_print_rational    (Rational);

Rational    add_rational            (Rational, Rational);
Rational    sub_rational            (Rational, Rational);
Rational    mul_rational            (Rational, Rational);
Rational    div_rational            (Rational, Rational);

#endif