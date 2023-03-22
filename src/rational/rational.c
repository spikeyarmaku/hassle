#include "rational.h"

Rational_t _make_rational() {
    Rational_t r = (Rational_t)allocate_mem("_make_rational", NULL, sizeof(struct _Rational));
    r->numerator = NULL;
    r->denominator = NULL;
    r->sign = 0;
    return r;
}

// A 0/0 is an invalid rational, and signifies an Error
BOOL is_valid_rational(Rational_t r) {
    return !(is_null_alnat(r->denominator) && is_null_alnat(r->numerator));
}

BOOL is_equal_rational(Rational_t r1, Rational_t r2) {
    return
        is_equal_alnat(r1->numerator, r2->numerator) &&
        is_equal_alnat(r1->denominator, r2->denominator) &&
        (r1->sign == r2->sign);
}

// Read a rational number from a string
Rational_t string_to_rational(char* string) {
    // Copy the string
    char* buf =
        strcpy(allocate_mem("string_to_rational", NULL, sizeof(char) * (strlen(string) + 1)), string);

    char* start = buf;
    Rational_t r = _make_rational();
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
    free_mem("string_to_rational", buf);
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

void free_rational(Rational_t r) {
    debug(2, "free_rational: numer...");
    free_alnat(r->numerator);
    debug(2, "done | denom...");
    free_alnat(r->denominator);
    debug(2, "done | rational...");
    free_mem("free_rational", r);
    debug(2, "done\n");
}

void simplify(Rational_t r) {
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alnat_t gcd = gcd_alnat(r->numerator, r->denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot simplify
        free_alnat(gcd);
        return;
    } else {
        // Simplify both by gcd
        Alnat_t simpl_numer = div_alnat(r->numerator, gcd);
        Alnat_t simpl_denom = div_alnat(r->denominator, gcd);
        int8_t sign = r->sign;
        free_alnat(r->numerator);
        free_alnat(r->denominator);
        r->numerator = simpl_numer;
        r->denominator = simpl_denom;
        r->sign = sign;
    }
    free_alnat(gcd);
}

void reciprocate(Rational_t r) {
    Alnat_t temp = r->numerator;
    r->numerator = r->denominator;
    r->denominator = temp;
}

int print_rational(char* buf, Rational_t r) {
    if (r->sign < 0) {
        printf("-");
    }
    char* alnat1 = debug_print_alnat(r->numerator);
    char* alnat2 = debug_print_alnat(r->denominator);
    int count = sprintf(buf, "%s / %s", alnat1, alnat2);
    free(alnat1); free(alnat2);
    return count;
}

void debug_print_rational(Rational_t r) {
    if (r->sign < 0) {
        printf("-");
    }
    char* alnat1 = debug_print_alnat(r->numerator);
    char* alnat2 = debug_print_alnat(r->denominator);
    printf("%s / %s", alnat1, alnat2);
    free(alnat1); free(alnat2);
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
Rational_t add_rational(Rational_t r1, Rational_t r2) {
    Rational_t r = _make_rational();
    Alnat_t n1 = mul_alnat(r1->numerator, r2->denominator);
    Alnat_t n2 = mul_alnat(r2->numerator, r1->denominator);
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

Rational_t sub_rational(Rational_t r1, Rational_t r2) {
    Rational_t r = _make_rational();
    Alnat_t n1 = mul_alnat(r1->numerator, r2->denominator);
    Alnat_t n2 = mul_alnat(r2->numerator, r1->denominator);
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

Rational_t mul_rational(Rational_t multiplicand, Rational_t multiplier) {
    Rational_t r = _make_rational();
    r->numerator = mul_alnat(multiplicand->numerator, multiplier->numerator);
    r->denominator =
        mul_alnat(multiplicand->denominator, multiplier->denominator);
    simplify(r);
    r->sign = multiplicand->sign * multiplier->sign;
    return r;
}

Rational_t div_rational(Rational_t dividend, Rational_t divisor) {
    reciprocate(divisor);
    Rational_t r = mul_rational(dividend, divisor);
    reciprocate(divisor);
    return r;
}
