#include "rational.h"

// A 0/0 is an invalid rational, and signifies an error
uint8_t is_valid_rational(struct Rational r) {
    return !(is_null_alnat(r._denominator) && is_null_alnat(r._numerator));
}

// Read a rational number from a string
struct Rational string_to_rational(char* string) {
    char* start = string;
    struct Rational r;
    r._numerator = NULL;
    r._denominator = NULL;
    if (*string == '-') {
        r._sign = -1;
    } else {
        r._sign = 1;
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
    //     r._sign = -1;
    //     r._numerator = make_null_alnat();
    //     r._denominator = r._numerator;
    //     return r;
    // }

    // Construct an integer number from the digits alone
    r._numerator = string_to_alnat(string);
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
        r._denominator = string_to_alnat(fraction_digits);
    } else {
        r._denominator = make_single_digit_alnat(1);
    }

    simplify(&r);
    return r;
}

void free_rational(struct Rational* r) {
    free_alnat(&(r->_numerator));
    free_alnat(&(r->_denominator));
    r->_numerator = NULL;
    r->_denominator = NULL;
    r->_sign = 0;
}

void simplify(struct Rational* r) {
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alnat gcd = gcd_alnat(r->_numerator, r->_denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot simplify
        free_alnat(&gcd);
        return;
    } else {
        // Simplify both by gcd
        Alnat simpl_numer = div_alnat(r->_numerator, gcd);
        Alnat simpl_denom = div_alnat(r->_denominator, gcd);
        int8_t sign = r->_sign;
        free_rational(r);
        r->_numerator = simpl_numer;
        r->_denominator = simpl_denom;
        r->_sign = sign;
    }
    free_alnat(&gcd);
}

void reciprocate(struct Rational* r) {
    Alnat temp = r->_numerator;
    r->_numerator = r->_denominator;
    r->_denominator = temp;
}

void debug_print_rational(struct Rational r) {
    if (r._sign < 0) {
        printf("-");
    }
    printf("%s / %s", debug_print_alnat(r._numerator),
        debug_print_alnat(r._denominator));
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
struct Rational add_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alnat n1 = mul_alnat(r1._numerator, r2._denominator);
    Alnat n2 = mul_alnat(r2._numerator, r1._denominator);
    if (r1._sign == r2._sign) {
        r._numerator = add_alnat(n1, n2);
        r._sign = r1._sign;
    } else {
        r._numerator = sub_alnat(n1, n2, &r._sign);
    }
    r._denominator = mul_alnat(r1._denominator, r2._denominator);
    simplify(&r);
    free_alnat(&n1);
    free_alnat(&n2);
    return r;
}

struct Rational sub_rational(struct Rational r1, struct Rational r2) {
    struct Rational r;
    Alnat n1 = mul_alnat(r1._numerator, r2._denominator);
    Alnat n2 = mul_alnat(r2._numerator, r1._denominator);
    if (r1._sign == r2._sign) {
        r._numerator = sub_alnat(n1, n2, &r._sign);
    } else {
        r._numerator = add_alnat(n1, n2);
        r._sign = r1._sign;
    }
    r._denominator = mul_alnat(r1._denominator, r2._denominator);
    simplify(&r);
    free_alnat(&n1);
    free_alnat(&n2);
    return r;
}

struct Rational mul_rational(
        struct Rational multiplicand, struct Rational multiplier) {
    struct Rational r;
    r._numerator = mul_alnat(multiplicand._numerator, multiplier._numerator);
    r._denominator = mul_alnat(multiplicand._denominator, multiplier._denominator);
    simplify(&r);
    r._sign = multiplicand._sign * multiplier._sign;
    return r;
}

struct Rational div_rational(
        struct Rational dividend, struct Rational divisor) {
    reciprocate(&divisor);
    struct Rational r = mul_rational(dividend, divisor);
    reciprocate(&divisor);
    return r;
}
