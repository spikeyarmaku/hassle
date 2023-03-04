#include "rational.h"

// A 0/0 is an invalid rational, and signifies an error
uint8_t is_valid_rational(struct Rational* r) {
    return !(is_null_alnat(r->denominator) && is_null_alnat(r->numerator));
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
    //     r.numerator = make_null_alnat();
    //     r.denominator = r.numerator;
    //     return r;
    // }

    // Construct an integer number from the digits alone
    r.numerator = string_to_alnat(string);
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
        r.denominator = string_to_alnat(fraction_digits);
    } else {
        r.denominator = make_single_digit_alnat(1);
    }

    simplify(&r);
    return r;
}

void destroy_rational(struct Rational r) {
    destroy_alnat(r.numerator);
    destroy_alnat(r.denominator);
}

void simplify(struct Rational* r) {
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alnat gcd = gcd_alnat(r->numerator, r->denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot simplify
        destroy_alnat(gcd);
        return;
    } else {
        // Simplify both by gcd
        Alnat simpl_numer = div_alnat(r->numerator, gcd);
        Alnat simpl_denom = div_alnat(r->denominator, gcd);
        destroy_rational(*r);
        r->numerator = simpl_numer;
        r->denominator = simpl_denom;
    }
    destroy_alnat(gcd);
}

void reciprocate(struct Rational* r) {
    Alnat temp = r->numerator;
    r->numerator = r->denominator;
    r->denominator = temp;
}

void debug_print_rational(struct Rational r) {
    if (r.sign < 0) {
        printf("-");
    }
    printf("%s / %s", debug_print_alnat(r.numerator),
        debug_print_alnat(r.denominator));
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
struct Rational add_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alnat n1 = mul_alnat(r1.numerator, r2.denominator);
    Alnat n2 = mul_alnat(r2.numerator, r1.denominator);
    if (r1.sign == r2.sign) {
        r.numerator = add_alnat(n1, n2);
        r.sign = r1.sign;
    } else {
        r.numerator = sub_alnat(n1, n2, &r.sign);
    }
    r.denominator = mul_alnat(r1.denominator, r2.denominator);
    simplify(&r);
    destroy_alnat(n1);
    destroy_alnat(n2);
    return r;
}

struct Rational sub_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alnat n1 = mul_alnat(r1.numerator, r2.denominator);
    Alnat n2 = mul_alnat(r2.numerator, r1.denominator);
    if (r1.sign == r2.sign) {
        r.numerator = sub_alnat(n1, n2, &r.sign);
    } else {
        r.numerator = add_alnat(n1, n2);
        r.sign = r1.sign;
    }
    r.denominator = mul_alnat(r1.denominator, r2.denominator);
    simplify(&r);
    destroy_alnat(n1);
    destroy_alnat(n2);
    return r;
}

struct Rational mul_rational(
        struct Rational multiplicand, struct Rational multiplier) {
    struct Rational r;
    r.numerator = mul_alnat(multiplicand.numerator, multiplier.numerator);
    r.denominator = mul_alnat(multiplicand.denominator, multiplier.denominator);
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
