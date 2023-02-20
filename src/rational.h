#ifndef _RATIONAL_H_
#define _RATIONAL_H_

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h> // DEBUG

#include "alint.h"

struct Rational {
    uint8_t sign;
    struct Alint* numerator;
    struct Alint* denominator; // If it is NULL, treat it as 1
};

struct Rational string_to_rational(char*);
void destroy_rational(struct Rational);
uint8_t is_valid_rational(struct Rational*);

#endif