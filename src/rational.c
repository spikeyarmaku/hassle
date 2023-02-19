#include "rational.h"

// TODO Support bases other than 10 (2, 8 and 16 would be nice)

// Construct an alint from a string containing a base-10 representation of a
// number
// The way it is done is to take the first n digits, that together form a number
// greater than 255, divide it with 256, take the quotient, and that will be the
// first byte. Then take the remainder, add enough characters to make it greater
// than 255, and repeat. It's like how they teach in elementary school, e.g.:
// 92537 : 256 = 361      361 : 256 = 1
// 1573                   105
//   377
//   121
struct Alint* string_to_alint(char* string) {
    printf("enter string_to_alint with arg %s\n", string);
    struct Alint* b = NULL;
    int intermediate = 0;
    char *current_char_ptr = string;
    char *next_pass_cursor = string;
    // TODO if the type of Alint's data changes, this needs to be changed, too
    int divider = 256;
    
    int go_on = 1;
    while (go_on) {
        printf("outer loop\n");
        struct Alint* b_inter = (struct Alint*)malloc(sizeof(struct Alint));
        // Read in enough digits to make the number greater than 255
        while (intermediate < divider) {
            intermediate = intermediate * 10 + *current_char_ptr - '0';
            printf("    reading char: %c, intermediate: %d\n",
                *current_char_ptr, intermediate);
            current_char_ptr++;
            // Divide it by `divider`, and get the remainder and quotient
            div_t result = div(intermediate, divider);
            printf("Division result: (%d / %d) = %d * %d + %d\n",
                intermediate, divider, result.quot, divider, result.rem);
            // Write the digits to the next pass, except for leading zeroes
            if (!(next_pass_cursor == string && result.quot == 0)) {
                *next_pass_cursor = result.quot + '0';
                next_pass_cursor++;
            }

            // If we reached the end of the string, go to the next pass
            if (*current_char_ptr == 0) {
                // If the read in value is less than the divider, we're finished
                if (intermediate < divider) {
                    go_on = 0;
                }
                // If we reach the end of the string, finish up
                break;
            }
            intermediate = result.rem;
        }
        b_inter->byte = intermediate;
        b_inter->next = b;
        b = b_inter;
        *next_pass_cursor = 0;
        current_char_ptr = string;
        next_pass_cursor = string;
        intermediate = 0;
        printf("String in memory: %s\n", string);
    }

    return b;
}

void debug_print_alint(struct Alint* alint) {
    while (alint != NULL) {
        printf("%d ", alint->byte);
        alint = alint->next;
    }
}

char* alint_to_string(struct Alint* alint) {
    //
}

void destroy_alint(struct Alint alint) {
    //
}

// Read a rational number from a string
struct Rational string_to_rational(char* string) {
    
    // Construct an integer number from the digits alone
    // Construct an integer (a power of 10) from the number of digits after the
    // decimal separator
    // Construct a rational from these two integers
}

void destroy_rational(struct Rational r) {
    //
}
