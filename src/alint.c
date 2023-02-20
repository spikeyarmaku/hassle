#include "alint.h"

// TODO Support bases other than 10 (2, 8 and 16 would be nice)

// Construct an alint from a string containing a base-10 representation of a
// number, disregarding any non-digit characters in the process
// The way it is done is to take the first n digits, that together form a number
// greater than 255, divide it with 256, take the quotient, and that will be the
// first byte. Then take the remainder, add enough characters to make it greater
// than 255, and repeat. It's like how they teach in elementary school, e.g.:
// 92537 : 256 = 361      361 : 256 = 1
// 1573                   105
//   377
//   121
struct Alint* string_to_alint(char* string) {
    struct Alint* b = NULL;
    struct Alint* b_counter = NULL;
    int intermediate = 0;
    char *current_char_ptr = string;
    char *next_pass_cursor = string;
    
    int go_on = 1;
    while (go_on) {
        struct Alint* b_inter = (struct Alint*)malloc(sizeof(struct Alint));
        // Read in enough digits to make the number greater than 255
        while (intermediate < ALINT_MAX) {
            while (!isdigit(*current_char_ptr)) {
                current_char_ptr++;
            }
            intermediate = intermediate * 10 + *current_char_ptr - '0';
            current_char_ptr++;
            // Divide it by `divider`, and get the remainder and quotient
            div_t result = div(intermediate, ALINT_MAX);
            // Write the digits to the next pass, except for leading zeroes
            if (!(next_pass_cursor == string && result.quot == 0)) {
                *next_pass_cursor = result.quot + '0';
                next_pass_cursor++;
            }

            // If we reached the end of the string, go to the next pass
            if (*current_char_ptr == 0) {
                // If the read in value is less than the divider, we're finished
                if (intermediate < ALINT_MAX) {
                    go_on = 0;
                }
                // If we reach the end of the string, finish up
                break;
            }
            intermediate = result.rem;
        }
        b_inter->num = intermediate;
        b_inter->next = NULL;
        if (b_counter == NULL) {
            b_counter = b_inter;
            b = b_counter;
        } else {
            b_counter->next = b_inter;
        }
        b_counter = b_inter;
        *next_pass_cursor = 0;
        current_char_ptr = string;
        next_pass_cursor = string;
        intermediate = 0;
    }

    return b;
}

void debug_print_alint(struct Alint* alint) {
    printf("<");
    while (alint != NULL) {
        printf("%d", alint->num);
        if (alint->next != NULL) {
            printf(" ");
        }
        alint = alint->next;
    }
    printf(">");
}

char* alint_to_string(struct Alint* alint) {
    // TODO
}

void destroy_alint(struct Alint* alint) {
    while (alint != NULL) {
        struct Alint* next = alint->next;
        free(alint);
        alint = next;
    }
}

struct Alint* make_null_alint() {
    struct Alint* alint = (struct Alint*)malloc(sizeof(struct Alint));
    alint->num = 0;
    alint->next = NULL;
    return alint;
}

uint8_t is_null_alint(struct Alint* alint) {
    return alint->num == 0 && alint->next == NULL;
}

struct Alint* add_alint(struct Alint* a1, struct Alint* a2) {
    struct Alint* result = NULL;
    struct Alint* pointer = NULL;
    uint8_t carry = 0;
    if (a1 == NULL && a2 == NULL) {
        return result;
    }

    carry = 0;
    while (a1 != NULL || a2 != NULL) {
        struct Alint* next_num = (struct Alint*)malloc(sizeof(struct Alint));
        next_num->next = NULL;
        ALINT_TYPE num1 = a1 == NULL ? 0 : a1->num;
        ALINT_TYPE num2 = a2 == NULL ? 0 : a2->num;

        // Check if the sum of the two is bigger than the highest value they can
        // hold
        next_num->num = (ALINT_TYPE)(num1 + num2 + carry);
        carry = next_num->num < num1 ? 1 : 0;

        // printf("num1: %d, num2: %d, result: %d, carry: %d\n",
        //     num1, num2, next_num->num, carry);

        if (a1 != NULL) {
            a1 = a1->next;
        }
        if (a2 != NULL) {
            a2 = a2->next;
        }
        if (pointer == NULL) {
            result = next_num;
            pointer = next_num;
        } else {
            pointer->next = next_num;
        }
        pointer = next_num;
    }

    return result;
}

struct Alint* make_complement_alint(struct Alint* alint) {
    struct Alint* start = alint;

    // Change the number in-place to be its own complement
    while (alint != NULL) {
        alint->num = (ALINT_MAX - 1) - alint->num;
        alint = alint->next;
    }

    // Add 1
    struct Alint* one = make_null_alint();
    one->num = 1;
    
    alint = start;
    struct Alint* result = add_alint(alint, one);
    free(one);

    // Change the number back
    while (alint != NULL) {
        alint->num = (ALINT_MAX - 1) - alint->num;
        alint = alint->next;
    }

    return result;
}

struct Alint* sub_alint(struct Alint* a1, struct Alint* a2) {
    uint8_t a1_gt_a2 = compare_alint(a1, a2) > 0;
    struct Alint* a1_new = a1_gt_a2 ? make_complement_alint(a1) : a1;
    struct Alint* a2_new = a1_gt_a2 ? a2 : make_complement_alint(a2);
    struct Alint* sum = add_alint(a1_new, a2_new);
    struct Alint* result = make_complement_alint(sum);
    free(sum);
    a1_gt_a2 ? free(a1_new) : free(a2_new);
    strip_alint(result);
    return result;
}

// Strips an alint from useless zeroes
void strip_alint(struct Alint* alint) {
    struct Alint* current = alint;
    struct Alint* prev = current;
    struct Alint* last_non_zero = current;

    // Find the start of the useless zeroes
    while (current != NULL) {
        if (current->num != 0) {
            last_non_zero = NULL;
        } else {
            if (prev->num != 0) {
                last_non_zero = prev;
            }
        }

        prev = current;
        current = current->next;
    }

    // Strip them
    if (last_non_zero != NULL) {
        destroy_alint(last_non_zero->next);
        last_non_zero->next = NULL;
    }
}

// Return 1 if first alint is greater, -1 if second is greater, 0 if equal
int8_t compare_alint(struct Alint* a1, struct Alint* a2) {
    uint8_t a1_gt_a2 = 0;
    while (a1 != NULL || a2 != NULL) {
        // If either pointer is null, whichever is not null is the bigger one
        int8_t a1null = a1 == NULL ? 0 : 1;
        int8_t a2null = a2 == NULL ? 0 : -1;
        if (a1null * a2null == 0) {
            return a1null + a2null;
        }

        if (a1->num != a2->num) {
            a1_gt_a2 = a1->num > a2->num ? 1 : -1;
        }

        // Else advance the pointers
        a1 = a1->next;
        a2 = a2->next;
    }
    return a1_gt_a2;
}

// Reduce by the GCD of two alints using the Euclidean method
// https://en.wikipedia.org/wiki/Euclidean_algorithm
void gcd(struct Alint* a1, struct Alint* a2) {
    // TODO
}

