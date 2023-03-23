#include "alnat.h"

struct AlnatBuilder _alnat_make_builder() {
    struct AlnatBuilder b;
    b.ptr = NULL;
    b.next = 0;
    b.size = 0;
    return b;
}

// Add a new block to the alnat's memory
enum ErrorCode _alnat_expand(struct AlnatBuilder* b) {
    size_t new_size = b->size + ALNAT_BUFFER_SIZE;
    Alnat_t new_ptr = NULL;
    new_ptr = (Alnat_t)allocate_mem("_alnat_expand", b->ptr, sizeof(uint8_t) * new_size);
    if (new_ptr == NULL) {
        error("_alnat_expand: Error while allocating.\n");
        return Error;
    }
    b->size = new_size;
    b->ptr = new_ptr;
    return Success;
}

// Add a new digit to alnat
enum ErrorCode _alnat_add_digit(uint8_t digit, struct AlnatBuilder* b) {
    // Check if the new digit is smaller than ALNAT_MAX
    if (digit >= ALNAT_MAX) {
        error("add_byte_to_alnat: digit %d exceeds the maximum (%d)\n", digit,
            ALNAT_MAX - 1);
        return Error;
    }

    // Check if the alnat has digits
    if (b->size != 0) {
        // If there is, set the last digit's mask to 1
        _alnat_unsafe_mark_digit(b->next - 1, 1, b->ptr);
    }
    // Check if we have enough space to extend the number with a new digit
    if (b->next == b->size) {
        if(_alnat_expand(b)) {
            error("add_byte_to_alnat: Error while expanding alnat.\n");
            return Error;
        }
    }
    b->ptr[b->next] = digit;
    b->next++;
    return Success;
}

// Free up unused memory and set last byte to 0
enum ErrorCode _alnat_finalize(struct AlnatBuilder* b) {
    debug(1, "_alnat_finalize\n");
    if (b->ptr != NULL) {
        Alnat_t new_ptr = allocate_mem("_alnat_finalize", b->ptr, sizeof(uint8_t) * b->next);
        if (new_ptr == NULL) {
            error("_alnat_finalize: Error while reallocating.\n");
            debug(1, "/_alnat_finalize\n");
            return Error;
        }
        b->ptr = new_ptr;
        b->size = b->next;
    }
    debug(1, "/_alnat_finalize\n");
    return Success;
}

struct AlnatMarcher _alnat_make_marcher(Alnat_t a) {
    struct AlnatMarcher m;
    m.ptr = a;
    m.counter = 0;
    m.finished = a == NULL ? TRUE : FALSE;
    return m;
}

uint8_t _alnat_get_next_digit(struct AlnatMarcher* m) {
    uint8_t d = 0;
    if (!_alnat_is_end(*m)) {
        d = _alnat_get_curr_digit(*m);
        _alnat_move_forward(m);
    }
    return d;
}

uint8_t _alnat_get_prev_digit(struct AlnatMarcher* m) {
    uint8_t d = _alnat_get_curr_digit(*m);
    _alnat_move_backward(m);
    return d;
}

uint8_t _alnat_get_curr_digit(struct AlnatMarcher m) {
    uint8_t b = m.ptr[m.counter];
    return b & ~ALNAT_MAX;
}

void _alnat_move_forward(struct AlnatMarcher* m) {
    if (!m->finished) {
        m->counter++;
        if ((m->ptr[m->counter] & ALNAT_MAX) == 0) {
            m->finished = TRUE;
        }
    }
}

void _alnat_move_backward(struct AlnatMarcher* m) {
    if (m->counter != 0) {
        m->counter--;
        m->finished = FALSE;
    }
}

BOOL _alnat_is_end(struct AlnatMarcher m) {
    return m.finished;
}

BOOL _alnat_is_start(struct AlnatMarcher m) {
    return m.counter == 0;
}

size_t _alnat_get_marcher_pos(struct AlnatMarcher m) {
    return m.counter;
}

void _alnat_rewind_marcher(struct AlnatMarcher* m) {
    *m = _alnat_make_marcher(m->ptr);
}

uint8_t _alnat_unsafe_get_digit(size_t n, Alnat_t a) {
    return a[n] & ~ALNAT_MAX;
}

uint8_t _alnat_unsafe_is_last_digit(size_t n, Alnat_t a) {
    return (a[n] & ALNAT_MAX) == ALNAT_MAX ? Success : Error;
}

void _alnat_unsafe_mark_digit(size_t n, uint8_t is_non_last, Alnat_t a) {
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
Alnat_t string_to_alnat(char* string) {
    debug(1, "string_to_alnat\n");
    struct AlnatBuilder b = _alnat_make_builder();
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
        
        if (_alnat_add_digit(intermediate, &b)) {
            error("string_to_alnat: couldn't add %d to alnat\n", intermediate);
            alnat_free(b.ptr);
            debug(1, "/string_to_alnat\n");
            return NULL;
        }
        *next_pass_cursor = 0;
        current_char_ptr = string;
        next_pass_cursor = string;
        intermediate = 0;
    }

    if (_alnat_finalize(&b)) {
        error("string_to_alnat: couldn't finalize alnat\n");
        alnat_free(b.ptr);
        debug(1, "/string_to_alnat\n");
        return NULL;
    }

    debug(1, "/string_to_alnat\n");
    return b.ptr;
}

// Double the digits in a string, least significant digit first. E.g. "2678"
// becomes "42571"
void _alnat_str_double(char* num_str) {
    INDEX index = 0;
    uint8_t d;
    uint8_t carry = 0;
    BOOL go_on = TRUE;
    while (go_on) {
        d = num_str[index];
        if (d == 0) {
            go_on = FALSE;
        } else {
            d -= '0';
        }

        d *= 2;
        d += carry;

        carry = d > 9 ? 1 : 0;
        d %= 10;

        num_str[index] = d + '0';
        index++;
    }
    if (num_str[index - 1] == '0') {
        num_str[index - 1] = 0;
    } else {
        num_str[index] = 0;
    }
}

// Add the second number to the first one, least significant digit first, and
// write it back into the first parameter. E.g. "456" and "28" becomes "637".
void _alnat_str_add(char* num_str, char* num2_str) {
    INDEX index = 0;
    BOOL go_on = TRUE;
    uint8_t d1;
    uint8_t d2;
    uint8_t d3;
    BOOL num1_end = FALSE;
    BOOL num2_end = FALSE;
    uint8_t carry = 0;
    while (go_on) {
        d1 = num1_end == TRUE ? 0 : num_str[index];
        d2 = num2_end == TRUE ? 0 : num2_str[index];

        if (d1 == 0) {
            num1_end = TRUE;
        } else {
            d1 -= '0';
        }
        if (d2 == 0) {
            num2_end = TRUE;
            go_on = FALSE;
        } else {
            d2 -= '0';
        }

        d3 = d1 + d2 + carry;
        carry = d3 > 9 ? 1 : 0;
        d3 %= 10;
        num_str[index] = d3 + '0';
        
        index++;
    }
}

char* alnat_to_string(Alnat_t alnat) {
    // The number of digits of a number N in base B can be calculated like this:
    // log(B, N) + 1
    // In this case, it is log(10, N) + 1.
    // The log can be calculated without actually calculating the number.
    // Since the number is in base 128, and we know the number of digits (D),
    // the number will be somewhere between 128^(D - 1) - 1 and 128^D - 1.
    // Therefore, the maximum value it can take is 128^D - 1.
    // log(10, 128^D) = log(10, 128) * D, which is 2.1072099696479 * D
    // In this case, we go with 2.2 just to be on the safe side
    size_t digit_base_128_count = 0;
    struct AlnatMarcher m = _alnat_make_marcher(alnat);

    // need to do a little bit of switcheroo, because the _alnat_is_start(m)
    // and _alnat_is_start(m) methods signal one digit too soon, so we need to
    // delay the end of the loop by one
    BOOL go_on = TRUE;
    BOOL is_finished = _alnat_is_end(m);
    while (go_on) {
        printf("%d | %llu | %d -> ", go_on, digit_base_128_count, is_finished);
        go_on = !is_finished; // switcheroo part 1
        _alnat_get_next_digit(&m);
        digit_base_128_count++;
        is_finished = _alnat_is_end(m); // switcheroo part 2
        printf("%d | %llu | %d\n", go_on, digit_base_128_count, is_finished);
    }
    
    size_t digit_base_10_count = ceil(2.2 * digit_base_128_count) + 2;
    char* result = (char*)allocate_mem("alnat_to_string", NULL,
        sizeof(char) * digit_base_10_count);
    if (result == NULL) return result;

    printf("%llu digits in base 128, allocated %llu digits\n",
        digit_base_128_count, digit_base_10_count);

    // Zero out the buffer
    for (INDEX i = 0; i < digit_base_10_count; i++) {
        result[i] = 0;
    }

    go_on = TRUE;
    is_finished = _alnat_is_start(m);
    while (go_on) {
        go_on = !is_finished; // switcheroo part 1
        // write the base-10 string repr of the next byte
        char byte_str[4], reverse_byte_str[4];
        sprintf(byte_str, "%d", _alnat_get_prev_digit(&m));
        uint8_t padding = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if (byte_str[i] == 0) {
                padding = 4 - i;
                break;
            }
        }
        for (uint8_t i = 0; i < 4 - padding; i++) {
            reverse_byte_str[i] = byte_str[3 - i - padding];
        }
        reverse_byte_str[4 - padding] = 0;
        _alnat_str_add(result, reverse_byte_str);
        // printf("Padding: %d | Written byte: %s | reverse: %s | Alnat: %s | f: %d | g: %d\n", padding, byte_str, reverse_byte_str, result, is_finished, go_on);

        // If we are not on the first digit already, multiply it by 2 seven
        // times (2^7 = 128)
        if (!is_finished) {
            for (uint8_t i = 0; i < 7; i++) {
                _alnat_str_double(result);
                // printf("Double #%d | Alnat: %s\n", i, result);
            }
        }
        is_finished = _alnat_is_start(m); // switcheroo part 2
    }

    // Strip the unused characters
    // Search for the first 0 byte
    char* current = result;
    INDEX first_zero = 0;
    for (INDEX i = 0; i < digit_base_10_count; i++) {
        if (current[i] == 0) {
            first_zero = i;
            break;
        }
    }
    printf("first zero: %llu\n", first_zero);
    // go back until the first non-0 *digit*
    for (INDEX i = first_zero - 1; i >= 0; i--) {
        if (current[i] != '0') {
            first_zero = i;
            break;
        }
    }
    printf("REALLOC %llu bytes\n", first_zero + 2);

    result = (char*)allocate_mem("alnat_to_string", result,
        sizeof(char) * (first_zero + 2));
    if (result == NULL) return result;
    result[first_zero + 1] = 0;

    return result;
}

void alnat_free(Alnat_t alnat) {
    free_mem("alnat_free", alnat);
}

Alnat_t alnat_make_single_digit(uint8_t digit) {
    Alnat_t alnat = (uint8_t*)allocate_mem("alnat_make_single_digit", NULL,
        sizeof(uint8_t));
    alnat[0] = digit;
    return alnat;
}

BOOL alnat_is_null(Alnat_t alnat) {
    return alnat[0] == 0;
}

BOOL alnat_is_equal(Alnat_t a1, Alnat_t a2) {
    struct AlnatMarcher m1 = _alnat_make_marcher(a1);
    struct AlnatMarcher m2 = _alnat_make_marcher(a2);
    while (!_alnat_is_end(m1) && !_alnat_is_end(m2)) {
        if (_alnat_get_next_digit(&m1) != _alnat_get_next_digit(&m2)) {
            return FALSE;
        }
    }
    return _alnat_is_end(m1) == _alnat_is_end(m2);
}

Alnat_t _alnat_copy(Alnat_t a) {
    Alnat_t result;
    size_t count = 0;
    do {
        count++;
    } while (!_alnat_unsafe_is_last_digit(count-1, a));
    result = (Alnat_t)allocate_mem("_alnat_copy", NULL, sizeof(uint8_t) * count);
    count = 0;
    do {
        result[count] = a[count];
        count++;
    } while (!_alnat_unsafe_is_last_digit(count-1, a));
    return result;
}

Alnat_t alnat_add(Alnat_t a1, Alnat_t a2) {
    struct AlnatBuilder b = _alnat_make_builder();
    struct AlnatMarcher m1 = _alnat_make_marcher(a1);
    struct AlnatMarcher m2 = _alnat_make_marcher(a2);
    uint8_t carry = 0;

    uint8_t a1digit = 0;
    uint8_t a2digit = 0;
    while (_alnat_is_end(m1) * _alnat_is_end(m2) == 0) {
        a1digit = _alnat_get_next_digit(&m1);
        a2digit = _alnat_get_next_digit(&m2);
        // Check if the sum of the two is bigger than the highest value they can
        // hold
        uint8_t next_byte = a1digit + a2digit + carry;
        carry = next_byte < ALNAT_MAX ? 0 : 1;
        next_byte = next_byte < ALNAT_MAX ? next_byte : next_byte - ALNAT_MAX;
        
        if (_alnat_add_digit(next_byte, &b)) {
            error("alnat_add: couldn't add %d to alnat\n", next_byte);
            alnat_free(b.ptr);
            return NULL;
        }
    }
    if (carry) {
        if (_alnat_add_digit(carry, &b)) {
            error("alnat_add: couldn't add %d to alnat\n", carry);
            alnat_free(b.ptr);
            return NULL;
        }
    }
    if (_alnat_finalize(&b)) {
        error("Error while finalizing alnat\n");
        alnat_free(b.ptr);
        return NULL;
    }

    return b.ptr;
}

Alnat_t alnat_sub(Alnat_t a1, Alnat_t a2, int8_t* sign) {
    int8_t a1_gt_a2 = _alnat_compare(a1, a2);
    if (a1_gt_a2 == 0) {
        return alnat_make_single_digit(0);
    } else {
        a1_gt_a2 = a1_gt_a2 > 0 ? 1 : 0;
        Alnat_t a1_new = a1_gt_a2 ? _alnat_make_complement(a1) : a1;
        Alnat_t a2_new = a1_gt_a2 ? a2 : _alnat_make_complement(a2);
        Alnat_t sum = alnat_add(a1_new, a2_new);
        Alnat_t result = _alnat_make_complement(sum);
        alnat_free(sum);
        a1_gt_a2 ? alnat_free(a1_new) : alnat_free(a2_new);
        _alnat_strip(&result);
        if (sign != NULL) {
            *sign = a1_gt_a2 ? 1 : -1;
        }
        return result;
    }
}

// TODO maybe there is a more efficient method
// https://en.wikipedia.org/wiki/Multiplication_algorithm
Alnat_t alnat_mul(Alnat_t multiplicand, Alnat_t multiplier) {
    Alnat_t result = alnat_make_single_digit(0);
    // Copy the multiplier
    Alnat_t multiplier_inter = _alnat_copy(multiplier);
    Alnat_t one = alnat_make_single_digit(1);

    while (!alnat_is_null(multiplier_inter)) {
        Alnat_t result_inter = alnat_add(result, multiplicand);
        alnat_free(result);
        result = result_inter;
        
        Alnat_t multiplier_inter_inter =
            alnat_sub(multiplier_inter, one, NULL);
        alnat_free(multiplier_inter);
        multiplier_inter = multiplier_inter_inter;
    }
    alnat_free(one);
    alnat_free(multiplier_inter);

    return result;
}

// TODO it could be made more efficient: instead of always adding `one`, these
// additions could be batched, e.g. add ALNAT_MAX-1 each time that much
// subtraction is done
// TODO maybe it could return the remainder, too?
Alnat_t alnat_div(Alnat_t dividend, Alnat_t divisor) {
    Alnat_t one = alnat_make_single_digit(1);

    if (_alnat_compare(dividend, divisor) == 0) {
        // If the dividend equals to the divisor, return 1
        return one;
    }

    Alnat_t result = alnat_make_single_digit(0);
    Alnat_t inter = _alnat_copy(dividend);
    while (_alnat_compare(inter, divisor) >= 0) {
        Alnat_t new_inter = alnat_sub(inter, divisor, NULL);
        alnat_free(inter);
        // printf("# %s ->", debug_print_alnat(new_inter));
        inter = new_inter;

        Alnat_t new_result = alnat_add(result, one);
        // printf("%s #\n", debug_print_alnat(result));
        alnat_free(result);
        result = new_result;
    }
    alnat_free(inter);
    alnat_free(one);

    return result;
}

Alnat_t _alnat_make_complement(Alnat_t alnat) {
    // Count the number of digits in the original alnat
    size_t size = 0;
    do {
        size++;
    } while (!_alnat_unsafe_is_last_digit(size-1, alnat));

    // Make an alnat of the same length
    Alnat_t result = (Alnat_t)allocate_mem("_alnat_make_complement", NULL, sizeof(uint8_t) * size);

    // Copy and complement the digits
    for (size_t i = 0; i < size; i++) {
        result[i] = alnat[i] ^ ~ALNAT_MAX;
    }

    // Add 1
    Alnat_t one = alnat_make_single_digit(1);
    Alnat_t new_result = alnat_add(result, one);
    
    alnat_free(result);
    result = new_result;
    alnat_free(one);

    return result;
}

// Strips an alnat from useless zeroes
void _alnat_strip(Alnat_t* alnat) {
    struct AlnatMarcher m = _alnat_make_marcher(*alnat);
    size_t first_useless_zero = 0;

    // Find the start of the useless zeroes
    while (!_alnat_is_end(m)) {
        if (_alnat_get_next_digit(&m) != 0) {
            first_useless_zero = _alnat_get_marcher_pos(m) + 1;
        }
    }
    uint8_t size = _alnat_get_marcher_pos(m) + 1;
    
    // Strip them
    if (first_useless_zero > 0 && first_useless_zero < size) {
        // printf("Resizing to %d\n", last_non_zero + 1);
        Alnat_t new_alnat = (Alnat_t)allocate_mem("_alnat_strip", *alnat, first_useless_zero);
        if (new_alnat == NULL) {
            error("_alnat_strip: Error while reallocating to %d\n", first_useless_zero);
            return;
        }
        *alnat = new_alnat;
        (*alnat)[first_useless_zero - 1] &= ~ALNAT_MAX;
    }
}

// Return 1 if first alnat is greater, -1 if second is greater, 0 if equal
int8_t _alnat_compare(Alnat_t a1, Alnat_t a2) {
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

        digit1 = _alnat_unsafe_get_digit(pointer, a1);
        digit2 = _alnat_unsafe_get_digit(pointer, a2);
        if (digit1 != digit2) {
            a1_gt_a2 = digit1 > digit2 ? 1 : -1;
        }

        a1null = _alnat_unsafe_is_last_digit(pointer, a1) ? 0 : 1;
        a2null = _alnat_unsafe_is_last_digit(pointer, a2) ? 0 : -1;

        // Else advance the pointers
        pointer++;
    } while (a1null || a2null);
    return a1_gt_a2;
}

// Reduce by the GCD of two alnats using the Euclidean method
// https://en.wikipedia.org/wiki/Euclidean_algorithm
Alnat_t alnat_gcd(Alnat_t a1, Alnat_t a2) {
    // First we check which number is greater
    int8_t a1_gt_a2 = _alnat_compare(a1, a2);
    Alnat_t greater = a1_gt_a2 ? _alnat_copy(a1) : _alnat_copy(a2);
    Alnat_t lesser = a1_gt_a2 ? _alnat_copy(a2) : _alnat_copy(a1);

    // Then we keep subtracting the lesser from the greater until the lesser
    // becomes greater
    while (!alnat_is_null(lesser)) {
        while (_alnat_compare(greater, lesser) >= 0) {
            Alnat_t new_greater = alnat_sub(greater, lesser, NULL);
            alnat_free(greater);
            greater = new_greater;
        }
        Alnat_t temp = greater;
        greater = lesser;
        lesser = temp;
    }
    alnat_free(lesser);
    // If we allow the return value to share memory address with either of the
    // inputs, there is a danger of double deleting
    return greater;
}

