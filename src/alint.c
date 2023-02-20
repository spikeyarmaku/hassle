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
        
// printf("New pass!\n");
        // Read the number, and provide the digits for the next pass
        while (intermediate < ALINT_MAX) {
            // Skip non-digit characters
            while (!isdigit(*current_char_ptr)) {
                current_char_ptr++;
            }
// printf("New char: %c\n",*current_char_ptr);
            intermediate = intermediate * 10 + *current_char_ptr - '0';
            current_char_ptr++;
            // Divide it by `divider`, and get the remainder and quotient
            div_t result = div(intermediate, ALINT_MAX);
            // Write the digits to the next pass, except for leading zeroes
            if (!(next_pass_cursor == string && result.quot == 0)) {
// printf("Writing %c, new intermediate: %d\n", result.quot + '0', result.rem);
                *next_pass_cursor = result.quot + '0';
                next_pass_cursor++;
            }

            // If we reached the end of the string, go to the next pass
            if (*current_char_ptr == 0) {
// printf("End of string reached, intermediate: %d\n", intermediate);
                // If the read in value is less than the divider, and we haven't
                // written any new digits for the next pass, we're finished
                if (intermediate < ALINT_MAX && next_pass_cursor == string) {
// printf("End reached (%d < %d)\n", intermediate, ALINT_MAX);
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
    long long int sum = 0;
    long long int place = 1;
    printf("<");
    while (alint != NULL) {
        sum += alint->num * place;
        place = place << 8;
        printf("%d", alint->num);
        if (alint->next != NULL) {
            printf(" ");
        }
        alint = alint->next;
    }
    printf("> (%d)\n", sum);
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

    if (carry) {
        struct Alint* next_num = (struct Alint*)malloc(sizeof(struct Alint));
        next_num->num = carry;
        next_num->next = NULL;
        pointer->next = next_num;
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

struct Alint* sub_alint(struct Alint* a1, struct Alint* a2, int8_t* sign) {
    int8_t a1_gt_a2 = compare_alint(a1, a2) > 0;
    struct Alint* a1_new = a1_gt_a2 ? make_complement_alint(a1) : a1;
    struct Alint* a2_new = a1_gt_a2 ? a2 : make_complement_alint(a2);
    struct Alint* sum = add_alint(a1_new, a2_new);
    struct Alint* result = make_complement_alint(sum);
    free(sum);
    a1_gt_a2 ? free(a1_new) : free(a2_new);
    strip_alint(result);
    if (sign != NULL) {
        *sign = a1_gt_a2 ? 1 : -1;
    }
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
    int8_t a1_gt_a2 = 0;
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
struct Alint* gcd_alint(struct Alint* a1, struct Alint* a2) {
    // First we check which number is greater
    int8_t a1_gt_a2 = compare_alint(a1, a2);
    struct Alint* greater = a1_gt_a2 ? a1 : a2;
    struct Alint* lesser = a1_gt_a2 ? a2 : a1;

    // Then we keep subtracting the lesser from the greater until the lesser
    // becomes greater
    while (!is_null_alint(lesser)) {
        printf("Greater: "); debug_print_alint(greater);
        printf("Lesser:  "); debug_print_alint(lesser);
        printf("\n");
        while (a1_gt_a2 >= 0) {
            struct Alint* new_greater = sub_alint(greater, lesser, &a1_gt_a2);
            if (greater != a1 && greater != a2) {
                free(greater);
            }
            greater = new_greater;
        }
        struct Alint* temp = greater;
        greater = lesser;
        lesser = temp;
    }
    if (lesser != a1 && lesser != a2) {
        free(lesser);
    }
    return greater;
}

// TODO it could be made more efficient: instead of always adding `one`, these
// additions could be batched, e.g. add ALINT_MAX-1 each time that much
// subtraction is done
// TODO maybe it could return the remainder, too?
struct Alint* div_alint(struct Alint* dividend, struct Alint* divisor) {
    struct Alint* result = make_null_alint();
    struct Alint* one = make_null_alint();
    one->num = 1;

    int counter = 0;
    int8_t a1_gt_a2 = compare_alint(dividend, divisor);

    struct Alint* inter = dividend;
    while (a1_gt_a2 >= 0) {
        struct Alint* new_inter = sub_alint(inter, divisor, &a1_gt_a2);
        a1_gt_a2 ? printf(" ") : printf("-");
        if (inter != dividend) {
            free(inter);
        }
        inter = new_inter;

        struct Alint* new_result = add_alint(result, one);
        free(result);
        result = new_result;
    }

    return result;
}
