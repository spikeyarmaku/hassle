#include "rational.h"

// A 0/0 with negative sign is an invalid rational, and signifies an error
uint8_t is_valid_rational(struct Rational* r) {
    return !(r->sign == -1 && is_null_alint(r->denominator) &&
        is_null_alint(r->numerator));
}

// Read a rational number from a string
struct Rational string_to_rational(char* string) {
    struct Rational r;
    if (*string == '-') {
        r.sign = -1;
    } else {
        r.sign = 1;
    }

    // Search for the first digit
    uint8_t counter = 0;
    char* fraction_digits = NULL; // Ptr to the first fractional digit
    while (counter < 2 && !isdigit(*string)) {
        if (*string == '.') {
            fraction_digits = string;
        }
        counter++;
        string++;
    }
    if (!isdigit(*string)) {
        r.sign = -1;
        r.numerator = make_null_alint();
        r.denominator = r.numerator;
        return r;
    }

    // Construct an integer number from the digits alone
    struct Alint* numerator = string_to_alint(string);
    // Construct an integer (a power of 10) from the number of digits after the
    // decimal separator
    // To do this, we overwrite the digits first
    char* nuller = fraction_digits;
    *nuller = 1;
    nuller++;
    while (*nuller != NULL) {
        *nuller = 0;
        nuller++;
    }
    struct Alint* denominator = string_to_alint(fraction_digits);

    // Find the greatest common divisor
    // Construct a rational from these two integers
}

void destroy_rational(struct Rational r) {
    // TODO
}
