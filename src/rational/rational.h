#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h> // DEBUG

#include "rational\alnat.h"

/*
Rational usage:

Construction:
  A Rational can be made from a base-10 string representation using
  `string_to_rational`.

Destruction:
  Call `free_rational`.
*/

struct Rational {
    int8_t _sign;
    Alnat _numerator;
    Alnat _denominator; // If it is NULL, treat it as 1
};

struct Rational string_to_rational      (char*);
void            free_rational           (struct Rational*);
BOOL            is_valid_rational       (struct Rational*);
void            simplify                (struct Rational*);
void            reciprocate             (struct Rational*);
void            debug_print_rational    (struct Rational);

struct Rational add_rational            (struct Rational, struct Rational);
struct Rational sub_rational            (struct Rational, struct Rational);
struct Rational mul_rational            (struct Rational, struct Rational);
struct Rational div_rational            (struct Rational, struct Rational);

#endif