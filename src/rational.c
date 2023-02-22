#include "rational.h"

// A 0/0 is an invalid rational, and signifies an error
uint8_t is_valid_rational(struct Rational* r) {
    return !(is_null_alint(r->denominator) && is_null_alint(r->numerator));
}

// Read a rational number from a string
struct Rational string_to_rational(char* string) {
    char* start = string;
    struct Rational r;
    r.numerator = NULL;
    r.denominator = NULL;
    if (*string == '-') {
        r.sign = -1;
    } else {
        r.sign = 1;
    }

    // Search for the first fraction digit
    long long int counter = 0;
    char* fraction_digits = NULL; // Ptr to the first fractional digit
    while (*string != 0) {
        if (*string == '.') {
            fraction_digits = string;
        }
        string++;

        if (fraction_digits != NULL) {
            counter++;
        }
    }
    string = start;
    // TODO check if the number is valid
    // if (!isdigit(*string)) {
    //     r.sign = -1;
    //     r.numerator = make_null_alint();
    //     r.denominator = r.numerator;
    //     return r;
    // }

    // Construct an integer number from the digits alone
    r.numerator = string_to_alint(string);
    // Construct an integer (a power of 10) from the number of digits after the
    // decimal separator
    // To do this, we overwrite the digits first
    if (fraction_digits != NULL) {
        char* nuller = fraction_digits;
        *nuller = '1';
        for (long long int i = 0; i < counter; i++) {
            nuller++;
            *nuller = '0';
        }
        *nuller = 0;
        r.denominator = string_to_alint(fraction_digits);
    } else {
        r.denominator = make_single_digit_alint(1);
    }

    simplify(&r);
    return r;
}

void destroy_rational(struct Rational r) {
    destroy_alint(r.numerator);
    destroy_alint(r.denominator);
}

void simplify(struct Rational* r) {
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alint gcd = gcd_alint(r->numerator, r->denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot simplify
        destroy_alint(gcd);
        return;
    } else {
        // Simplify both by gcd
        Alint simpl_numer = div_alint(r->numerator, gcd);
        Alint simpl_denom = div_alint(r->denominator, gcd);
        destroy_rational(*r);
        r->numerator = simpl_numer;
        r->denominator = simpl_denom;
    }
    destroy_alint(gcd);
}

void reciprocate(struct Rational* r) {
    Alint temp = r->numerator;
    r->numerator = r->denominator;
    r->denominator = temp;
}

void debug_print_rational(struct Rational r) {
    if (r.sign < 0) {
        printf("-");
    }
    printf("%s / %s", debug_print_alint(r.numerator),
        debug_print_alint(r.denominator));
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
struct Rational add_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alint n1 = mul_alint(r1.numerator, r2.denominator);
    Alint n2 = mul_alint(r2.numerator, r1.denominator);
    if (r1.sign == r2.sign) {
        r.numerator = add_alint(n1, n2);
        r.sign = r1.sign;
    } else {
        r.numerator = sub_alint(n1, n2, &r.sign);
    }
    r.denominator = mul_alint(r1.denominator, r2.denominator);
    simplify(&r);
    destroy_alint(n1);
    destroy_alint(n2);
    return r;
}

struct Rational sub_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alint n1 = mul_alint(r1.numerator, r2.denominator);
    Alint n2 = mul_alint(r2.numerator, r1.denominator);
    if (r1.sign == r2.sign) {
        r.numerator = sub_alint(n1, n2, &r.sign);
    } else {
        r.numerator = add_alint(n1, n2);
        r.sign = r1.sign;
    }
    r.denominator = mul_alint(r1.denominator, r2.denominator);
    simplify(&r);
    destroy_alint(n1);
    destroy_alint(n2);
    return r;
}

struct Rational mul_rational(
        struct Rational multiplicand, struct Rational multiplier) {
    struct Rational r;
    r.numerator = mul_alint(multiplicand.numerator, multiplier.numerator);
    r.denominator = mul_alint(multiplicand.denominator, multiplier.denominator);
    simplify(&r);
    r.sign = multiplicand.sign * multiplier.sign;
    return r;
}

struct Rational div_rational(
        struct Rational dividend, struct Rational divisor) {
    reciprocate(&divisor);
    struct Rational r = mul_rational(dividend, divisor);
    reciprocate(&divisor);
    return r;
}
