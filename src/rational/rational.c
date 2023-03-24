#include "rational.h"

Rational_t _rational_make() {
    Rational_t r = (Rational_t)allocate_mem("_rational_make", NULL, sizeof(struct _Rational));
    r->numerator = NULL;
    r->denominator = NULL;
    r->sign = 0;
    return r;
}

// A 0/0 is an invalid rational, and signifies an Error
BOOL rational_is_valid(Rational_t r) {
    return !(alnat_is_null(r->denominator) && alnat_is_null(r->numerator));
}

BOOL rational_is_equal(Rational_t r1, Rational_t r2) {
    return
        alnat_is_equal(r1->numerator, r2->numerator) &&
        alnat_is_equal(r1->denominator, r2->denominator) &&
        (r1->sign == r2->sign);
}

// Read a rational number from a string
Rational_t string_to_rational(char* string) {
    // Copy the string
    char* buf =
        strcpy(allocate_mem("string_to_rational", NULL,
            sizeof(char) * (strlen(string) + 1)), string);

    char* start = buf;
    Rational_t r = _rational_make();
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
    
    // Construct an integer (a power of 10) from the number of digits after the
    // decimal separator
    // To do this, we overwrite the digits first
    if (fraction_digits != NULL) {
        char* nuller = fraction_digits;
        *nuller = '1';
        for (size_t i = 0; i < counter; i++) {
            nuller++;
            *nuller = '0';
        }
        *nuller = 0;
        r->denominator = string_to_alnat(fraction_digits);
    } else {
        r->denominator = alnat_make_single_digit(1);
    }

    free_mem("string_to_rational", buf);
    rational_simplify(r);

    return r;
}

void rational_free(Rational_t r) {
    alnat_free(r->numerator);
    alnat_free(r->denominator);
    free_mem("rational_free", r);
}

void rational_simplify(Rational_t r) {
    debug(1, "rational_simplify\n");
    // Find the greatest common divisor
    // printf("<<< Simplify "); debug_print_rational(*r);
    Alnat_t gcd = alnat_gcd(r->numerator, r->denominator);
    // printf("Rational simplified. >>>\n");
    
    // Construct a rational from these two integers
    if (gcd[0] == 1) {
        // Relative primes, cannot rational_simplify
        alnat_free(gcd);
        debug(-1, "/rational_simplify\n");
        return;
    } else {
        // Simplify both by gcd
        Alnat_t simpl_numer = alnat_div(r->numerator, gcd);
        Alnat_t simpl_denom = alnat_div(r->denominator, gcd);
        int8_t sign = r->sign;
        alnat_free(r->numerator);
        alnat_free(r->denominator);
        r->numerator = simpl_numer;
        r->denominator = simpl_denom;
        r->sign = sign;
    }
    alnat_free(gcd);
    debug(-1, "/rational_simplify\n");
}

void rational_reciprocate(Rational_t r) {
    Alnat_t temp = r->numerator;
    r->numerator = r->denominator;
    r->denominator = temp;
}

char* rational_to_string(Rational_t r) {
    char* alnat1 = alnat_to_string(r->numerator);
    char* alnat2 = alnat_to_string(r->denominator);
    // negative sign + first number + space + / + space + second number +
    // terminating 0
    size_t length =
        ((r->sign == -1) ? 1 : 0) + strlen(alnat1) + 3 + strlen(alnat2) + 1;
    char* rational_str = (char*)allocate_mem("rational_to_string", NULL,
        sizeof(char) * length);
    memset(rational_str, 0, sizeof(char) * length);
    if (r->sign == -1) {
        strcat(rational_str, "-");
    }
    strcat(rational_str, alnat1);
    strcat(rational_str, " / ");
    strcat(rational_str, alnat2);

    free_mem("rational_to_string", alnat1);
    free_mem("rational_to_string", alnat2);
    return rational_str;
}

// TODO don't just blindly multiply, perhaps calculating the LCM is better
Rational_t rational_add(Rational_t r1, Rational_t r2) {
    Rational_t r = _rational_make();
    Alnat_t n1 = alnat_mul(r1->numerator, r2->denominator);
    Alnat_t n2 = alnat_mul(r2->numerator, r1->denominator);
    if (r1->sign == r2->sign) {
        r->numerator = alnat_add(n1, n2);
        r->sign = r1->sign;
    } else {
        r->numerator = alnat_sub(n1, n2, &(r->sign));
    }
    r->denominator = alnat_mul(r1->denominator, r2->denominator);
    rational_simplify(r);
    alnat_free(n1);
    alnat_free(n2);
    return r;
}

Rational_t rational_sub(Rational_t r1, Rational_t r2) {
    Rational_t r = _rational_make();
    Alnat_t n1 = alnat_mul(r1->numerator, r2->denominator);
    Alnat_t n2 = alnat_mul(r2->numerator, r1->denominator);
    if (r1->sign == r2->sign) {
        r->numerator = alnat_sub(n1, n2, &(r->sign));
    } else {
        r->numerator = alnat_add(n1, n2);
        r->sign = r1->sign;
    }
    r->denominator = alnat_mul(r1->denominator, r2->denominator);
    rational_simplify(r);
    alnat_free(n1);
    alnat_free(n2);
    return r;
}

Rational_t rational_mul(Rational_t multiplicand, Rational_t multiplier) {
    Rational_t r = _rational_make();
    r->numerator = alnat_mul(multiplicand->numerator, multiplier->numerator);
    r->denominator =
        alnat_mul(multiplicand->denominator, multiplier->denominator);
    rational_simplify(r);
    r->sign = multiplicand->sign * multiplier->sign;
    return r;
}

Rational_t rational_div(Rational_t dividend, Rational_t divisor) {
    rational_reciprocate(divisor);
    Rational_t r = rational_mul(dividend, divisor);
    rational_reciprocate(divisor);
    return r;
}
