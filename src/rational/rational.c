#include "rational.h"

Rational _make_rational() {
    Rational r = (Rational)allocate_mem(NULL, sizeof(struct _Rational));
    r->numerator = NULL;
    r->denominator = NULL;
    r->sign = 0;
    return r;
}

// A 0/0 is an invalid rational, and signifies an error
uint8_t is_valid_rational(Rational r) {
    return !(is_null_alnat(r->denominator) && is_null_alnat(r->numerator));
}

// Read a rational number from a string
Rational string_to_rational(char* string) {
    // Copy the string
    char* buf =
        strcpy(allocate_mem(NULL, sizeof(char) * (strlen(string) + 1)), string);

    char* start = buf;
    Rational r = _make_rational();
    if (*buf == '-') {
        r->sign = -1;
    } else {
        r->sign = 1;
    }

    // Search for the first fraction digit
    long long int counter = 0;
    char* fraction_digits = NULL; // Ptr to the first fractional digit
    while (*buf != 0) {
        if (*buf == '.') {
            fraction_digits = buf;
        }
        buf++;

        if (fraction_digits != NULL) {
            counter++;
        }
    }
    buf = start;
    // TODO check if the number is valid
    // if (!isdigit(*string)) {
    //     r.sign = -1;
    //     r.numerator = make_null_alnat();
    //     r.denominator = r.numerator;
    //     return r;
    // }

    // Construct an integer number from the digits alone
    r->numerator = string_to_alnat(buf);
    free_mem(buf);
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
        r->denominator = string_to_alnat(fraction_digits);
    } else {
        r->denominator = make_single_digit_alnat(1);
    }

    simplify(r);
    return r;
}

void free_rational(Rational r) {
    free_alnat(r->numerator);
    free_alnat(r->denominator);
    free_mem(r);
}

void simplify(Rational r) {
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alnat gcd = gcd_alnat(r->numerator, r->denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot simplify
        free_alnat(gcd);
        return;
    } else {
        // Simplify both by gcd
        Alnat simpl_numer = div_alnat(r->numerator, gcd);
        Alnat simpl_denom = div_alnat(r->denominator, gcd);
        int8_t sign = r->sign;
        free_alnat(r->numerator);
        free_alnat(r->denominator);
        r->numerator = simpl_numer;
        r->denominator = simpl_denom;
        r->sign = sign;
    }
    free_alnat(gcd);
}

void reciprocate(Rational r) {
    Alnat temp = r->numerator;
    r->numerator = r->denominator;
    r->denominator = temp;
}

void debug_print_rational(Rational r) {
    if (r->sign < 0) {
        printf("-");
    }
    printf("%s / %s", debug_print_alnat(r->numerator),
        debug_print_alnat(r->denominator));
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
Rational add_rational(Rational r1, Rational r2) {
    Rational r = _make_rational();
    Alnat n1 = mul_alnat(r1->numerator, r2->denominator);
    Alnat n2 = mul_alnat(r2->numerator, r1->denominator);
    if (r1->sign == r2->sign) {
        r->numerator = add_alnat(n1, n2);
        r->sign = r1->sign;
    } else {
        r->numerator = sub_alnat(n1, n2, &(r->sign));
    }
    r->denominator = mul_alnat(r1->denominator, r2->denominator);
    simplify(r);
    free_alnat(n1);
    free_alnat(n2);
    return r;
}

Rational sub_rational(Rational r1, Rational r2) {
    Rational r = _make_rational();
    Alnat n1 = mul_alnat(r1->numerator, r2->denominator);
    Alnat n2 = mul_alnat(r2->numerator, r1->denominator);
    if (r1->sign == r2->sign) {
        r->numerator = sub_alnat(n1, n2, &(r->sign));
    } else {
        r->numerator = add_alnat(n1, n2);
        r->sign = r1->sign;
    }
    r->denominator = mul_alnat(r1->denominator, r2->denominator);
    simplify(r);
    free_alnat(n1);
    free_alnat(n2);
    return r;
}

Rational mul_rational(Rational multiplicand, Rational multiplier) {
    Rational r = _make_rational();
    r->numerator = mul_alnat(multiplicand->numerator, multiplier->numerator);
    r->denominator =
        mul_alnat(multiplicand->denominator, multiplier->denominator);
    simplify(r);
    r->sign = multiplicand->sign * multiplier->sign;
    return r;
}

Rational div_rational(Rational dividend, Rational divisor) {
    reciprocate(divisor);
    Rational r = mul_rational(dividend, divisor);
    reciprocate(divisor);
    return r;
}
