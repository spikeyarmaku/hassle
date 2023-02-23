#include "alint.h"

void debug_print_alint_builder(struct AlintBuilder b) {
    printf("AlintBuilder:\n  ptr:  %llu\n  size: %d\n  next: %d\n",
        b._ptr, b._size, b._next);
}

struct AlintBuilder create_alint_builder() {
    struct AlintBuilder b;
    b._ptr = NULL;
    b._next = 0;
    b._size = 0;
    return b;
}

Alint get_alint(struct AlintBuilder b) {
    return b._ptr;
}

// Add a new block to the alint's memory
ErrorCode expand_alint(struct AlintBuilder* b) {
    size_t new_size = b->_size + ALINT_BUFFER_SIZE;
    Alint new_ptr = NULL;
    if (b->_ptr == NULL) {
        new_ptr = (Alint)malloc(sizeof(uint8_t) * new_size);
    } else {
        new_ptr = (Alint)realloc(b->_ptr, sizeof(uint8_t) * new_size);
    }
    if (new_ptr == NULL) {
        error("expand_alint: error while allocating.\n");
        return 1;
    }
    b->_size = new_size;
    b->_ptr = new_ptr;
    return 0;
}

// Add a new digit to alint
ErrorCode add_digit_to_alint(uint8_t digit, struct AlintBuilder* b) {
    // Check if the new digit is smaller than ALINT_MAX
    if (digit >= ALINT_MAX) {
        error("add_byte_to_alint: digit %d exceeds the maximum (%d)\n", digit,
            ALINT_MAX - 1);
        return 1;
    }

    // Check if the alint has digits
    if (b->_size != 0) {
        // If there is, set the last digit's mask to 1
        unsafe_mark_digit_alint(b->_next - 1, 1, b->_ptr);
    }
    // Check if we have enough space to extend the number with a new digit
    if (b->_next == b->_size) {
        if(expand_alint(b)) {
            error("add_byte_to_alint: error while expanding alint.\n");
            return 1;
        }
    }
    b->_ptr[b->_next] = digit;
    b->_next++;
    return 0;
}

// Free up unused memory and set last byte to 0
ErrorCode finalize_alint(struct AlintBuilder* b) {
    if (b->_ptr != NULL) {
        Alint new_ptr = realloc(b->_ptr, sizeof(uint8_t) * b->_next);
        if (new_ptr == NULL) {
            error("finalize_alint: error while reallocating.\n");
            return 1;
        }
        b->_ptr = new_ptr;
        b->_size = b->_next;
    }
    return 0;
}

struct AlintMarcher create_alint_marcher(Alint a) {
    struct AlintMarcher m;
    m._ptr = a;
    m._counter = 0;
    m._finished = a == NULL ? 1 : 0;
    return m;
}

uint8_t get_next_alint_digit(struct AlintMarcher* m) {
    if (m->_finished) {
        return 0;
    } else {
        uint8_t b = m->_ptr[m->_counter];
        if ((b & ALINT_MAX) == ALINT_MAX) {
            m->_counter++;
        } else {
            m->_finished = 1;
        }
        return b & ~ALINT_MAX;
    }
}

uint8_t is_end_of_alint(struct AlintMarcher m) {
    return m._finished;
}

size_t get_marcher_position(struct AlintMarcher m) {
    return m._counter;
}

void rewind_marcher(struct AlintMarcher* m) {
    *m = create_alint_marcher(m->_ptr);
}

ErrorCode unsafe_get_digit_alint(size_t n, Alint a) {
    return a[n] & ~ALINT_MAX;
}

ErrorCode unsafe_is_last_digit_alint(size_t n, Alint a) {
    return (a[n] & ALINT_MAX) == ALINT_MAX ? 0 : 1;
}

void unsafe_mark_digit_alint(size_t n, uint8_t is_non_last, Alint a) {
    if (is_non_last) {
        a[n] |= ALINT_MAX;
    } else {
        a[n] &= ~ALINT_MAX;
    }
}

// TODO Support bases other than 10 (2, 8 and 16 would be nice)

// Construct an alint from a string containing a base-10 representation of a
// number, disregarding any non-digit characters in the process
// The way it is done is to take the first n digits, that together form a number
// greater than ALINT_MAX-1, divide it with ALINT_MAX, take the quotient, and
// that will be the first digit. Then take the remainder, add enough characters
// to make it greater than ALINT_MAX-1, and repeat. It's like how they teach in
// elementary school, e.g. if ALINT_MAX is 256, the result is 121 105 1:
// 92537 : 256 = 361      361 : 256 = 1   1 : 256 = 0
// 1573                   105             1
//   377
//   121
Alint string_to_alint(char* string) {
    struct AlintBuilder b = create_alint_builder();
    uint16_t intermediate = 0;
    char *current_char_ptr = string;
    char *next_pass_cursor = string;

    int go_on = 1;
    while (go_on) {
        // Read the number, and provide the digits for the next pass
        while (intermediate < ALINT_MAX) {
            // Skip non-digit characters
            while (!isdigit(*current_char_ptr)) {
                current_char_ptr++;
            }
            intermediate = intermediate * 10 + *current_char_ptr - '0';
            current_char_ptr++;
            // Divide it by `divider`, and get the remainder and quotient
            div_t div_result = div(intermediate, ALINT_MAX);
            // Write the digits to the next pass, except for leading zeroes
            if (!(next_pass_cursor == string && div_result.quot == 0)) {
                *next_pass_cursor = div_result.quot + '0';
                next_pass_cursor++;
            }

            // If we reached the end of the string, go to the next pass
            if (*current_char_ptr == 0) {
                // If the read in value is less than the divider, and we haven't
                // written any new digits for the next pass, we're finished
                if (intermediate < ALINT_MAX && next_pass_cursor == string) {
                    go_on = 0;
                }
                // If we reach the end of the string, finish up
                intermediate = div_result.rem;
                break;
            }
            intermediate = div_result.rem;
        }
        
        if (add_digit_to_alint(intermediate, &b)) {
            error("string_to_alint: couldn't add %d to alint\n", intermediate);
            destroy_alint(get_alint(b));
            return NULL;
        }
        *next_pass_cursor = 0;
        current_char_ptr = string;
        next_pass_cursor = string;
        intermediate = 0;
    }

    if (finalize_alint(&b)) {
        error("string_to_alint: couldn't finalize alint\n");
        destroy_alint(get_alint(b));
        return NULL;
    }

    return get_alint(b);
}

// Only works with alints that fit into an uint64_t
char* debug_print_alint(Alint alint) {
    if (alint == NULL) {
        return NULL;
    }
    uint64_t sum = 0;
    uint64_t place = 1;
    size_t current = 0;
    do {
        sum += unsafe_get_digit_alint(current, alint) * place;
        place = place * ALINT_MAX;
        current++;
    } while (!unsafe_is_last_digit_alint(current-1, alint));
    char* result = (char*)malloc(sizeof(char) * 30);
    sprintf(result, "%llu", sum);
    return result;
}

char* alint_to_string(Alint alint) {
    // TODO
}

Alint destroy_alint(Alint alint) {
    if (alint != NULL) {
        free(alint);
    }
    return NULL;
}

Alint make_single_digit_alint(uint8_t digit) {
    Alint alint = (uint8_t*)malloc(sizeof(uint8_t));
    alint[0] = digit;
    return alint;
}

uint8_t is_null_alint(Alint alint) {
    return alint[0] == 0;
}

Alint copy_alint(Alint a) {
    Alint result;
    size_t count = 0;
    do {
        count++;
    } while (!unsafe_is_last_digit_alint(count-1, a));
    result = (Alint)malloc(sizeof(uint8_t) * count);
    count = 0;
    do {
        result[count] = a[count];
        count++;
    } while (!unsafe_is_last_digit_alint(count-1, a));
    return result;
}

Alint add_alint(Alint a1, Alint a2) {
    struct AlintBuilder b = create_alint_builder();
    struct AlintMarcher m1 = create_alint_marcher(a1);
    struct AlintMarcher m2 = create_alint_marcher(a2);
    uint8_t carry = 0;

    uint8_t a1digit = 0;
    uint8_t a2digit = 0;
    while (is_end_of_alint(m1) * is_end_of_alint(m2) == 0) {
        uint8_t a1digit = get_next_alint_digit(&m1);
        uint8_t a2digit = get_next_alint_digit(&m2);
        // Check if the sum of the two is bigger than the highest value they can
        // hold
        uint8_t next_byte = a1digit + a2digit + carry;
        carry = next_byte < ALINT_MAX ? 0 : 1;
        next_byte = next_byte < ALINT_MAX ? next_byte : next_byte - ALINT_MAX;
        
        if (add_digit_to_alint(next_byte, &b)) {
            error("add_alint: couldn't add %d to alint\n", next_byte);
            destroy_alint(get_alint(b));
            return NULL;
        }
    }
    if (carry) {
        if (add_digit_to_alint(carry, &b)) {
            error("add_alint: couldn't add %d to alint\n", carry);
            destroy_alint(get_alint(b));
            return NULL;
        }
    }
    if (finalize_alint(&b)) {
        error("Error while finalizing alint\n");
        destroy_alint(get_alint(b));
        return NULL;
    }

    return get_alint(b);
}

Alint sub_alint(Alint a1, Alint a2, int8_t* sign) {
    int8_t a1_gt_a2 = compare_alint(a1, a2);
    if (a1_gt_a2 == 0) {
        return make_single_digit_alint(0);
    } else {
        a1_gt_a2 = a1_gt_a2 > 0 ? 1 : 0;
        Alint a1_new = a1_gt_a2 ? make_complement_alint(a1) : a1;
        Alint a2_new = a1_gt_a2 ? a2 : make_complement_alint(a2);
        Alint sum = add_alint(a1_new, a2_new);
        Alint result = make_complement_alint(sum);
        destroy_alint(sum);
        a1_gt_a2 ? destroy_alint(a1_new) : destroy_alint(a2_new);
        strip_alint(&result);
        if (sign != NULL) {
            *sign = a1_gt_a2 ? 1 : -1;
        }
        return result;
    }
}

// TODO maybe there is a more efficient method
// https://en.wikipedia.org/wiki/Multiplication_algorithm
Alint mul_alint(Alint multiplicand, Alint multiplier) {
    Alint result = make_single_digit_alint(0);
    // Copy the multiplier
    Alint multiplier_inter = copy_alint(multiplier);
    Alint one = make_single_digit_alint(1);

    while (!is_null_alint(multiplier_inter)) {
        Alint result_inter = add_alint(result, multiplicand);
        destroy_alint(result);
        result = result_inter;
        
        Alint multiplier_inter_inter =
            sub_alint(multiplier_inter, one, NULL);
        destroy_alint(multiplier_inter);
        multiplier_inter = multiplier_inter_inter;
    }
    destroy_alint(one);
    destroy_alint(multiplier_inter);

    return result;
}

// TODO it could be made more efficient: instead of always adding `one`, these
// additions could be batched, e.g. add ALINT_MAX-1 each time that much
// subtraction is done
// TODO maybe it could return the remainder, too?
Alint div_alint(Alint dividend, Alint divisor) {
    Alint one = make_single_digit_alint(1);

    if (compare_alint(dividend, divisor) == 0) {
        // If the dividend equals to the divisor, return 1
        return one;
    }

    Alint result = make_single_digit_alint(0);
    Alint inter = copy_alint(dividend);
    while (compare_alint(inter, divisor) >= 0) {
        Alint new_inter = sub_alint(inter, divisor, NULL);
        destroy_alint(inter);
        // printf("# %s ->", debug_print_alint(new_inter));
        inter = new_inter;

        Alint new_result = add_alint(result, one);
        // printf("%s #\n", debug_print_alint(result));
        destroy_alint(result);
        result = new_result;
    }
    destroy_alint(inter);
    destroy_alint(one);

    return result;
}

Alint make_complement_alint(Alint alint) {
    // Count the number of digits in the original alint
    size_t size = 0;
    do {
        size++;
    } while (!unsafe_is_last_digit_alint(size-1, alint));

    // Make an alint of the same length
    Alint result = (Alint)malloc(sizeof(uint8_t) * size);

    // Copy and complement the digits
    for (size_t i = 0; i < size; i++) {
        result[i] = alint[i] ^ ~ALINT_MAX;
    }

    // Add 1
    Alint one = make_single_digit_alint(1);
    Alint new_result = add_alint(result, one);
    
    destroy_alint(result);
    result = new_result;
    destroy_alint(one);

    return result;
}

// Strips an alint from useless zeroes
void strip_alint(Alint* alint) {
    struct AlintMarcher m = create_alint_marcher(*alint);
    size_t current = 0;
    size_t first_useless_zero = 0;

    // Find the start of the useless zeroes
    while (!is_end_of_alint(m)) {
        if (get_next_alint_digit(&m) != 0) {
            first_useless_zero = get_marcher_position(m) + 1;
        }
    }
    uint8_t size = get_marcher_position(m) + 1;
    
    // Strip them
    if (first_useless_zero > 0 && first_useless_zero < size) {
        // printf("Resizing to %d\n", last_non_zero + 1);
        Alint new_alint = (Alint)realloc(*alint, first_useless_zero);
        if (new_alint == NULL) {
            error("strip_alint: error while reallocating to %d\n", first_useless_zero);
            return;
        }
        *alint = new_alint;
        (*alint)[first_useless_zero - 1] &= ~ALINT_MAX;
    }
}

// Return 1 if first alint is greater, -1 if second is greater, 0 if equal
int8_t compare_alint(Alint a1, Alint a2) {
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

        digit1 = unsafe_get_digit_alint(pointer, a1);
        digit2 = unsafe_get_digit_alint(pointer, a2);
        if (digit1 != digit2) {
            a1_gt_a2 = digit1 > digit2 ? 1 : -1;
        }

        a1null = unsafe_is_last_digit_alint(pointer, a1) ? 0 : 1;
        a2null = unsafe_is_last_digit_alint(pointer, a2) ? 0 : -1;

        // Else advance the pointers
        pointer++;
    } while (a1null || a2null);
    return a1_gt_a2;
}

// Reduce by the GCD of two alints using the Euclidean method
// https://en.wikipedia.org/wiki/Euclidean_algorithm
Alint gcd_alint(Alint a1, Alint a2) {
    // First we check which number is greater
    int8_t a1_gt_a2 = compare_alint(a1, a2);
    Alint greater = a1_gt_a2 ? copy_alint(a1) : copy_alint(a2);
    Alint lesser = a1_gt_a2 ? copy_alint(a2) : copy_alint(a1);

    // Then we keep subtracting the lesser from the greater until the lesser
    // becomes greater
    while (!is_null_alint(lesser)) {
        while (compare_alint(greater, lesser) >= 0) {
            Alint new_greater = sub_alint(greater, lesser, NULL);
            destroy_alint(greater);
            greater = new_greater;
        }
        Alint temp = greater;
        greater = lesser;
        lesser = temp;
    }
    destroy_alint(lesser);
    // If we allow the return value to share memory address with either of the
    // inputs, there is a danger of double deleting
    return greater;
}

