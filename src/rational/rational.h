#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h> // DEBUG

#include "rational\alnat.h"

struct Rational {
    int8_t sign;
    Alnat numerator;
    Alnat denominator; // If it is NULL, treat it as 1
};

struct Rational string_to_rational      (char*);
void            destroy_rational        (struct Rational);
BOOL            is_valid_rational       (struct Rational*);
void            simplify                (struct Rational*);
void            reciprocate             (struct Rational*);
void            debug_print_rational    (struct Rational);

struct Rational add_rational            (struct Rational, struct Rational);
struct Rational sub_rational            (struct Rational, struct Rational);
struct Rational mul_rational            (struct Rational, struct Rational);
struct Rational div_rational            (struct Rational, struct Rational);

#endif