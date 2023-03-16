#include "alnat.h"

struct AlnatBuilder create_alnat_builder() {
    struct AlnatBuilder b;
    b._ptr = NULL;
    b._next = 0;
    b._size = 0;
    return b;
}

// Add a new block to the alnat's memory
ErrorCode expand_alnat(struct AlnatBuilder* b) {
    size_t new_size = b->_size + ALNAT_BUFFER_SIZE;
    Alnat new_ptr = NULL;
    new_ptr = (Alnat)allocate_mem(b->_ptr, sizeof(uint8_t) * new_size);
    if (new_ptr == NULL) {
        error("expand_alnat: error while allocating.\n");
        return ERROR;
    }
    b->_size = new_size;
    b->_ptr = new_ptr;
    return SUCCESS;
}

// Add a new digit to alnat
ErrorCode add_digit_to_alnat(uint8_t digit, struct AlnatBuilder* b) {
    // Check if the new digit is smaller than ALNAT_MAX
    if (digit >= ALNAT_MAX) {
        error("add_byte_to_alnat: digit %d exceeds the maximum (%d)\n", digit,
            ALNAT_MAX - 1);
        return ERROR;
    }

    // Check if the alnat has digits
    if (b->_size != 0) {
        // If there is, set the last digit's mask to 1
        unsafe_mark_digit_alnat(b->_next - 1, 1, b->_ptr);
    }
    // Check if we have enough space to extend the number with a new digit
    if (b->_next == b->_size) {
        if(expand_alnat(b)) {
            error("add_byte_to_alnat: error while expanding alnat.\n");
            return ERROR;
        }
    }
    b->_ptr[b->_next] = digit;
    b->_next++;
    return SUCCESS;
}

// Free up unused memory and set last byte to 0
ErrorCode finalize_alnat(struct AlnatBuilder* b) {
    if (b->_ptr != NULL) {
        Alnat new_ptr = allocate_mem(b->_ptr, sizeof(uint8_t) * b->_next);
        if (new_ptr == NULL) {
            error("finalize_alnat: error while reallocating.\n");
            return ERROR;
        }
        b->_ptr = new_ptr;
        b->_size = b->_next;
    }
    return SUCCESS;
}

struct AlnatMarcher create_alnat_marcher(Alnat a) {
    struct AlnatMarcher m;
    m._ptr = a;
    m._counter = 0;
    m._finished = a == NULL ? 1 : 0;
    return m;
}

uint8_t get_next_alnat_digit(struct AlnatMarcher* m) {
    if (m->_finished) {
        return 0;
    } else {
        uint8_t b = m->_ptr[m->_counter];
        if ((b & ALNAT_MAX) == ALNAT_MAX) {
            m->_counter++;
        } else {
            m->_finished = 1;
        }
        return b & ~ALNAT_MAX;
    }
}

BOOL is_end_of_alnat(struct AlnatMarcher m) {
    return m._finished;
}

size_t get_marcher_position(struct AlnatMarcher m) {
    return m._counter;
}

void rewind_marcher(struct AlnatMarcher* m) {
    *m = create_alnat_marcher(m->_ptr);
}

uint8_t unsafe_get_digit_alnat(size_t n, Alnat a) {
    return a[n] & ~ALNAT_MAX;
}

ErrorCode unsafe_is_last_digit_alnat(size_t n, Alnat a) {
    return (a[n] & ALNAT_MAX) == ALNAT_MAX ? SUCCESS : ERROR;
}

void unsafe_mark_digit_alnat(size_t n, uint8_t is_non_last, Alnat a) {
    if (is_non_last) {
        a[n] |= ALNAT_MAX;
    } else {
        a[n] &= ~ALNAT_MAX;
    }
}

// TODO Support bases other than 10 (2, 8 and 16 would be nice)

// Construct an alnat from a string containing a base-10 representation of a
// number, disregarding any non-digit characters in the process
// The way it is done is to take the first n digits, that together form a number
// greater than ALNAT_MAX-1, divide it with ALNAT_MAX, take the quotient, and
// that will be the first digit. Then take the remainder, add enough characters
// to make it greater than ALNAT_MAX-1, and repeat. It's like how they teach in
// elementary school, e.g. if ALNAT_MAX is 256, the result is 121 105 1:
// 92537 : 256 = 361      361 : 256 = 1   1 : 256 = 0
// 1573                   105             1
//   377
//   121
Alnat string_to_alnat(char* string) {
    struct AlnatBuilder b = create_alnat_builder();
    uint16_t intermediate = 0;
    char *current_char_ptr = string;
    char *next_pass_cursor = string;

    int go_on = 1;
    while (go_on) {
        // Read the number, and provide the digits for the next pass
        while (intermediate < ALNAT_MAX) {
            // Skip non-digit characters
            while (!isdigit(*current_char_ptr)) {
                current_char_ptr++;
            }
            intermediate = intermediate * 10 + *current_char_ptr - '0';
            current_char_ptr++;
            // Divide it by `divider`, and get the remainder and quotient
            div_t div_result = div(intermediate, ALNAT_MAX);
            // Write the digits to the next pass, except for leading zeroes
            if (!(next_pass_cursor == string && div_result.quot == 0)) {
                *next_pass_cursor = div_result.quot + '0';
                next_pass_cursor++;
            }

            // If we reached the end of the string, go to the next pass
            if (*current_char_ptr == 0) {
                // If the read in value is less than the divider, and we haven't
                // written any new digits for the next pass, we're finished
                if (intermediate < ALNAT_MAX && next_pass_cursor == string) {
                    go_on = 0;
                }
                // If we reach the end of the string, finish up
                intermediate = div_result.rem;
                break;
            }
            intermediate = div_result.rem;
        }
        
        if (add_digit_to_alnat(intermediate, &b)) {
            error("string_to_alnat: couldn't add %d to alnat\n", intermediate);
            free_alnat(&(b._ptr));
            return NULL;
        }
        *next_pass_cursor = 0;
        current_char_ptr = string;
        next_pass_cursor = string;
        intermediate = 0;
    }

    if (finalize_alnat(&b)) {
        error("string_to_alnat: couldn't finalize alnat\n");
        free_alnat(&(b._ptr));
        return NULL;
    }

    return b._ptr;
}

// Only works with alnats that fit into an uint64_t
char* debug_print_alnat(Alnat alnat) {
    if (alnat == NULL) {
        return NULL;
    }
    uint64_t sum = 0;
    uint64_t place = 1;
    size_t current = 0;
    do {
        sum += unsafe_get_digit_alnat(current, alnat) * place;
        place = place * ALNAT_MAX;
        current++;
    } while (!unsafe_is_last_digit_alnat(current-1, alnat));
    char* result = (char*)malloc(sizeof(char) * 30);
    sprintf(result, "%llu", sum);
    return result;
}

char* alnat_to_string(Alnat alnat) {
    // TODO
    return NULL;
}

void free_alnat(Alnat* alnat) {
    if (*alnat != NULL) {
        free_mem(*alnat);
    }
    *alnat = NULL;
}

Alnat make_single_digit_alnat(uint8_t digit) {
    Alnat alnat = (uint8_t*)allocate_mem(NULL, sizeof(uint8_t));
    alnat[0] = digit;
    return alnat;
}

BOOL is_null_alnat(Alnat alnat) {
    return alnat[0] == 0;
}

Alnat copy_alnat(Alnat a) {
    Alnat result;
    size_t count = 0;
    do {
        count++;
    } while (!unsafe_is_last_digit_alnat(count-1, a));
    result = (Alnat)allocate_mem(NULL, sizeof(uint8_t) * count);
    count = 0;
    do {
        result[count] = a[count];
        count++;
    } while (!unsafe_is_last_digit_alnat(count-1, a));
    return result;
}

Alnat add_alnat(Alnat a1, Alnat a2) {
    struct AlnatBuilder b = create_alnat_builder();
    struct AlnatMarcher m1 = create_alnat_marcher(a1);
    struct AlnatMarcher m2 = create_alnat_marcher(a2);
    uint8_t carry = 0;

    uint8_t a1digit = 0;
    uint8_t a2digit = 0;
    while (is_end_of_alnat(m1) * is_end_of_alnat(m2) == 0) {
        a1digit = get_next_alnat_digit(&m1);
        a2digit = get_next_alnat_digit(&m2);
        // Check if the sum of the two is bigger than the highest value they can
        // hold
        uint8_t next_byte = a1digit + a2digit + carry;
        carry = next_byte < ALNAT_MAX ? 0 : 1;
        next_byte = next_byte < ALNAT_MAX ? next_byte : next_byte - ALNAT_MAX;
        
        if (add_digit_to_alnat(next_byte, &b)) {
            error("add_alnat: couldn't add %d to alnat\n", next_byte);
            free_alnat(&(b._ptr));
            return NULL;
        }
    }
    if (carry) {
        if (add_digit_to_alnat(carry, &b)) {
            error("add_alnat: couldn't add %d to alnat\n", carry);
            free_alnat(&(b._ptr));
            return NULL;
        }
    }
    if (finalize_alnat(&b)) {
        error("Error while finalizing alnat\n");
        free_alnat(&(b._ptr));
        return NULL;
    }

    return b._ptr;
}

Alnat sub_alnat(Alnat a1, Alnat a2, int8_t* sign) {
    int8_t a1_gt_a2 = compare_alnat(a1, a2);
    if (a1_gt_a2 == 0) {
        return make_single_digit_alnat(0);
    } else {
        a1_gt_a2 = a1_gt_a2 > 0 ? 1 : 0;
        Alnat a1_new = a1_gt_a2 ? make_complement_alnat(a1) : a1;
        Alnat a2_new = a1_gt_a2 ? a2 : make_complement_alnat(a2);
        Alnat sum = add_alnat(a1_new, a2_new);
        Alnat result = make_complement_alnat(sum);
        free_alnat(&sum);
        a1_gt_a2 ? free_alnat(&a1_new) : free_alnat(&a2_new);
        strip_alnat(&result);
        if (sign != NULL) {
            *sign = a1_gt_a2 ? 1 : -1;
        }
        return result;
    }
}

// TODO maybe there is a more efficient method
// https://en.wikipedia.org/wiki/Multiplication_algorithm
Alnat mul_alnat(Alnat multiplicand, Alnat multiplier) {
    Alnat result = make_single_digit_alnat(0);
    // Copy the multiplier
    Alnat multiplier_inter = copy_alnat(multiplier);
    Alnat one = make_single_digit_alnat(1);

    while (!is_null_alnat(multiplier_inter)) {
        Alnat result_inter = add_alnat(result, multiplicand);
        free_alnat(&result);
        result = result_inter;
        
        Alnat multiplier_inter_inter =
            sub_alnat(multiplier_inter, one, NULL);
        free_alnat(&multiplier_inter);
        multiplier_inter = multiplier_inter_inter;
    }
    free_alnat(&one);
    free_alnat(&multiplier_inter);

    return result;
}

// TODO it could be made more efficient: instead of always adding `one`, these
// additions could be batched, e.g. add ALNAT_MAX-1 each time that much
// subtraction is done
// TODO maybe it could return the remainder, too?
Alnat div_alnat(Alnat dividend, Alnat divisor) {
    Alnat one = make_single_digit_alnat(1);

    if (compare_alnat(dividend, divisor) == 0) {
        // If the dividend equals to the divisor, return 1
        return one;
    }

    Alnat result = make_single_digit_alnat(0);
    Alnat inter = copy_alnat(dividend);
    while (compare_alnat(inter, divisor) >= 0) {
        Alnat new_inter = sub_alnat(inter, divisor, NULL);
        free_alnat(&inter);
        // printf("# %s ->", debug_print_alnat(new_inter));
        inter = new_inter;

        Alnat new_result = add_alnat(result, one);
        // printf("%s #\n", debug_print_alnat(result));
        free_alnat(&result);
        result = new_result;
    }
    free_alnat(&inter);
    free_alnat(&one);

    return result;
}

Alnat make_complement_alnat(Alnat alnat) {
    // Count the number of digits in the original alnat
    size_t size = 0;
    do {
        size++;
    } while (!unsafe_is_last_digit_alnat(size-1, alnat));

    // Make an alnat of the same length
    Alnat result = (Alnat)allocate_mem(NULL, sizeof(uint8_t) * size);

    // Copy and complement the digits
    for (size_t i = 0; i < size; i++) {
        result[i] = alnat[i] ^ ~ALNAT_MAX;
    }

    // Add 1
    Alnat one = make_single_digit_alnat(1);
    Alnat new_result = add_alnat(result, one);
    
    free_alnat(&result);
    result = new_result;
    free_alnat(&one);

    return result;
}

// Strips an alnat from useless zeroes
void strip_alnat(Alnat* alnat) {
    struct AlnatMarcher m = create_alnat_marcher(*alnat);
    size_t first_useless_zero = 0;

    // Find the start of the useless zeroes
    while (!is_end_of_alnat(m)) {
        if (get_next_alnat_digit(&m) != 0) {
            first_useless_zero = get_marcher_position(m) + 1;
        }
    }
    uint8_t size = get_marcher_position(m) + 1;
    
    // Strip them
    if (first_useless_zero > 0 && first_useless_zero < size) {
        // printf("Resizing to %d\n", last_non_zero + 1);
        Alnat new_alnat = (Alnat)allocate_mem(*alnat, first_useless_zero);
        if (new_alnat == NULL) {
            error("strip_alnat: error while reallocating to %d\n", first_useless_zero);
            return;
        }
        *alnat = new_alnat;
        (*alnat)[first_useless_zero - 1] &= ~ALNAT_MAX;
    }
}

// Return 1 if first alnat is greater, -1 if second is greater, 0 if equal
int8_t compare_alnat(Alnat a1, Alnat a2) {
    int8_t a1_gt_a2 = 0;
    size_t pointer = 0;
    int8_t a1null = 1;
    int8_t a2null = -1;
    uint8_t digit1;
    uint8_t digit2;
    do {
        // If either pointer is null, whichever is not null is the bigger one
        if (a1null * a2null == 0) {
            if (a1null + a2null == 0) {
                return a1_gt_a2;
            } else {
                return a1null + a2null;
            }
        }

        digit1 = unsafe_get_digit_alnat(pointer, a1);
        digit2 = unsafe_get_digit_alnat(pointer, a2);
        if (digit1 != digit2) {
            a1_gt_a2 = digit1 > digit2 ? 1 : -1;
        }

        a1null = unsafe_is_last_digit_alnat(pointer, a1) ? 0 : 1;
        a2null = unsafe_is_last_digit_alnat(pointer, a2) ? 0 : -1;

        // Else advance the pointers
        pointer++;
    } while (a1null || a2null);
    return a1_gt_a2;
}

// Reduce by the GCD of two alnats using the Euclidean method
// https://en.wikipedia.org/wiki/Euclidean_algorithm
Alnat gcd_alnat(Alnat a1, Alnat a2) {
    // First we check which number is greater
    int8_t a1_gt_a2 = compare_alnat(a1, a2);
    Alnat greater = a1_gt_a2 ? copy_alnat(a1) : copy_alnat(a2);
    Alnat lesser = a1_gt_a2 ? copy_alnat(a2) : copy_alnat(a1);

    // Then we keep subtracting the lesser from the greater until the lesser
    // becomes greater
    while (!is_null_alnat(lesser)) {
        while (compare_alnat(greater, lesser) >= 0) {
            Alnat new_greater = sub_alnat(greater, lesser, NULL);
            free_alnat(&greater);
            greater = new_greater;
        }
        Alnat temp = greater;
        greater = lesser;
        lesser = temp;
    }
    free_alnat(&lesser);
    // If we allow the return value to share memory address with either of the
    // inputs, there is a danger of double deleting
    return greater;
}

