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
    debug(1, "_alnat_add_digit\n");
    // Check if the new digit is smaller than ALNAT_MAX
    if (digit >= ALNAT_MAX) {
        error("add_byte_to_alnat: digit %d exceeds the maximum (%d)\n", digit,
            ALNAT_MAX - 1);
        debug(-1, "/_alnat_add_digit!\n");
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
            debug(-1, "/_alnat_add_digit!\n");
            return Error;
        }
    }
    b->ptr[b->next] = digit;
    b->next++;
    debug(-1, "/_alnat_add_digit\n");
    return Success;
}

// Free up unused memory and set last byte to 0
enum ErrorCode _alnat_finalize(struct AlnatBuilder* b) {
    debug(1, "_alnat_finalize\n");
    if (b->ptr != NULL) {
        Alnat_t new_ptr = allocate_mem("_alnat_finalize", b->ptr, sizeof(uint8_t) * b->next);
        if (new_ptr == NULL) {
            error("_alnat_finalize: Error while reallocating.\n");
            debug(-1, "/_alnat_finalize\n");
            return Error;
        }
        b->ptr = new_ptr;
        b->size = b->next;
    }
    debug(-1, "/_alnat_finalize\n");
    return Success;
}

struct AlnatMarcher _alnat_make_marcher(Alnat_t a) {
    struct AlnatMarcher m;
    m.ptr = a;
    m.counter = 0;
    return m;
}

uint8_t _alnat_get_next_digit(struct AlnatMarcher* m) {
    _alnat_move_forward(m);
    uint8_t d = _alnat_get_curr_digit(*m);
    return d;
}

uint8_t _alnat_get_prev_digit(struct AlnatMarcher* m) {
    _alnat_move_backward(m);
    uint8_t d = _alnat_get_curr_digit(*m);
    return d;
}

uint8_t _alnat_get_curr_digit(struct AlnatMarcher m) {
    uint8_t b = m.ptr[m.counter];
    return b & ~ALNAT_MAX;
}

BOOL _alnat_move_forward(struct AlnatMarcher* m) {
    if (!_alnat_is_end(*m)) {
        m->counter++;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL _alnat_move_backward(struct AlnatMarcher* m) {
    if (m->counter != 0) {
        m->counter--;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL _alnat_is_start(struct AlnatMarcher m) {
    return m.counter == 0;
}

BOOL _alnat_is_end(struct AlnatMarcher m) {
    return (m.ptr[m.counter] & ALNAT_MAX) == 0;
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

BOOL _alnat_unsafe_is_last_digit(size_t n, Alnat_t a) {
    return (a[n] & ALNAT_MAX) == ALNAT_MAX ? FALSE : TRUE;
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
    debug(1, "string_to_alnat - %s\n", string);
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
            debug(-1, "/string_to_alnat\n");
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
        debug(-1, "/string_to_alnat\n");
        return NULL;
    }

    debug(-1, "/string_to_alnat\n");
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
void _alnat_str_add(char* num1_str, char* num2_str) {
    INDEX index = 0;
    BOOL go_on = TRUE;
    uint8_t d1;
    uint8_t d2;
    uint8_t d3;
    BOOL num1_end = num1_str[0] == 0 ? TRUE : FALSE;
    BOOL num2_end = num2_str[0] == 0 ? TRUE : FALSE;
    uint8_t carry = 0;
    while (go_on) {
        d1 = num1_end == TRUE ? 0 : num1_str[index];
        d2 = num2_end == TRUE ? 0 : num2_str[index];

        if (d1 == 0) {
            num1_end = TRUE;
        } else {
            d1 -= '0';
        }
        if (d2 == 0) {
            num2_end = TRUE;
        } else {
            d2 -= '0';
        }

        if (num2_end && (carry == 0)) {
            go_on = FALSE;
        }

        d3 = d1 + d2 + carry;
        carry = d3 > 9 ? 1 : 0;
        d3 %= 10;
        num1_str[index] = d3 + '0';
        
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

    do {
        digit_base_128_count++;
    } while (_alnat_move_forward(&m));
    
    size_t digit_base_10_count = ceil(2.2 * digit_base_128_count) + 2;
    char* result = (char*)allocate_mem("alnat_to_string/result", NULL,
        sizeof(char) * digit_base_10_count);
    if (result == NULL) return result;

    // printf("%llu digits in base 128, allocated %llu digits\n",
    //     digit_base_128_count, digit_base_10_count);

    // Zero out the buffer
    memset(result, 0, digit_base_10_count);

    char byte_str[4], reverse_byte_str[4];
    do {
        // write the base-10 string repr of the next byte
        sprintf(byte_str, "%d", _alnat_get_curr_digit(m));
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
        // printf("Padding: %d | Written byte: %s | reverse: %s | Alnat: %s\n",
        //     padding, byte_str, reverse_byte_str, result);

        // If we are not on the first digit already, multiply it by 2 seven
        // times (2^7 = 128)
        if (!_alnat_is_start(m)) {
            for (uint8_t i = 0; i < 7; i++) {
                _alnat_str_double(result);
                // printf("Double #%d | Alnat: %s\n", i, result);
            }
        }
    } while (_alnat_move_backward(&m));

    // Strip the unused characters
    // Search for the first 0 byte
    char* current = result;
    INDEX length = 0;
    for (INDEX i = 0; i < digit_base_10_count; i++) {
        if (current[i] == 0) {
            length = i;
            break;
        }
    }
    // printf("first zero: %llu\n", length);
    // go back until the first non-0 *digit*
    for (INDEX i = length - 1; i > 0; i--) {
        if (current[i] != '0') {
            length = i + 2;
            break;
        }
    }
    // printf("REALLOC %llu bytes\n", length);

    result = (char*)allocate_mem("alnat_to_string/result/realloc", result,
        sizeof(char) * length);
    if (result == NULL) return result;
    result[length - 1] = 0;

    // Reverse the string
    uint8_t temp;
    for (size_t i = 0; i < (length - 2 - i); i++) {
        temp = result[i];
        result[i] = result[length - 2 - i];
        result[length - 2 - i] = temp;
    }

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
    BOOL a1end, a2end;
    do {
        if (_alnat_get_curr_digit(m1) != _alnat_get_curr_digit(m2)) {
            return FALSE;
        }
        a1end = _alnat_move_forward(&m1);
        a2end = _alnat_move_forward(&m2);
    } while (a1end && a2end);
    return a1end == a2end;
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
    debug(1, "alnat_add\n");
    
    // alnat_print_bytes(a1); printf(" + "); alnat_print_bytes(a2); printf("\n");
    
    struct AlnatBuilder b = _alnat_make_builder();
    struct AlnatMarcher m1 = _alnat_make_marcher(a1);
    struct AlnatMarcher m2 = _alnat_make_marcher(a2);
    uint8_t carry = 0;

    uint8_t a1digit = 0;
    uint8_t a2digit = 0;

    BOOL a1end = FALSE;
    BOOL a2end = FALSE;
    while (!(a1end && a2end)) {
        a1digit = a1end ? 0 : _alnat_get_curr_digit(m1);
        a2digit = a2end ? 0 : _alnat_get_curr_digit(m2);
        // Check if the sum of the two is bigger than the highest value they can
        // hold
        uint8_t next_byte = a1digit + a2digit + carry;
        carry = next_byte < ALNAT_MAX ? 0 : 1;
        next_byte = next_byte < ALNAT_MAX ? next_byte : next_byte - ALNAT_MAX;
        
        // printf("digit: %d + %d + %d = %d\n", a1digit, a2digit, carry, next_byte);
        if (_alnat_add_digit(next_byte, &b)) {
            error("alnat_add: couldn't add %d to alnat\n", next_byte);
            alnat_free(b.ptr);
            debug(-1, "/alnat_add!\n");
            return NULL;
        }
        
        a1end = !_alnat_move_forward(&m1);
        a2end = !_alnat_move_forward(&m2);
        // printf("a1end: %d, a2end: %d\n", a1end, a2end);
    };

    if (carry) {
        if (_alnat_add_digit(carry, &b)) {
            error("alnat_add: couldn't add %d to alnat\n", carry);
            alnat_free(b.ptr);
            debug(-1, "/alnat_add!\n");
            return NULL;
        }
    }
    if (_alnat_finalize(&b)) {
        error("Error while finalizing alnat\n");
        alnat_free(b.ptr);
        debug(-1, "/alnat_add!\n");
        return NULL;
    }

    debug(-1, "/alnat_add\n");
    return b.ptr;
}

Alnat_t alnat_sub(Alnat_t a1, Alnat_t a2, int8_t* sign) {
    debug(1, "alnat_sub\n");
    
    // alnat_print(a1); printf("("); alnat_print_bytes(a1); printf(") - ");
    // alnat_print(a2); printf("("); alnat_print_bytes(a2); printf(")\n");
    
    int8_t a1_gt_a2 = _alnat_compare(a1, a2);
    if (a1_gt_a2 == 0) {
        debug(-1, "/alnat_sub\n");
        return alnat_make_single_digit(0);
    } else {
        a1_gt_a2 = a1_gt_a2 > 0 ? 1 : 0;
        Alnat_t a1_new = a1_gt_a2 ? _alnat_make_complement(a1) : a1;
        
        // printf("Compl.: "); alnat_print(a1_new); printf("(");
        // alnat_print_bytes(a1_new); printf(")\n");
        
        Alnat_t a2_new = a1_gt_a2 ? a2 : _alnat_make_complement(a2);
        Alnat_t sum = alnat_add(a1_new, a2_new);
        Alnat_t result = _alnat_make_complement(sum);
        alnat_free(sum);
        a1_gt_a2 ? alnat_free(a1_new) : alnat_free(a2_new);
        _alnat_strip(&result);
        if (sign != NULL) {
            *sign = a1_gt_a2 ? 1 : -1;
        }
    
        // printf("RESULT: "); alnat_print(result); printf(" -- "); printf("\n");
    
        debug(-1, "/alnat_sub\n");
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
    debug(1, "alnat_div\n");
    Alnat_t one = alnat_make_single_digit(1);

    if (_alnat_compare(dividend, divisor) == 0) {
        // If the dividend equals to the divisor, return 1
        debug(-1, "/alnat_div\n");
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

    debug(-1, "alnat_div\n");
    return result;
}

Alnat_t _alnat_make_complement(Alnat_t alnat) {
    debug(1, "_alnat_make_complement\n");
    // Count the number of digits in the original alnat
    size_t size = 1;
    while (!_alnat_unsafe_is_last_digit(size - 1, alnat)) {
        size++;
    }

    // debug(0, "digits: %d\n", size);

    // Make an alnat of the same length
    Alnat_t result = (Alnat_t)allocate_mem("_alnat_make_complement", NULL,
        sizeof(uint8_t) * size);

    // Copy and complement the digits
    for (size_t i = 0; i < size; i++) {
        // ^ is bitwise XOR, ~ALNAT_MAX is 01111111
        // Basically it switches all bits except for the first one
        result[i] = alnat[i] ^ ~ALNAT_MAX;
        // debug(0, "complementing %d to %d\n", alnat[i], result[i]);
    }

    // Add 1
    Alnat_t one = alnat_make_single_digit(1);
    Alnat_t new_result = alnat_add(result, one);
    
    // debug(0, "new result: ");
    // for (size_t i = 0; i < size; i++) {
    //     debug(0, "%d ", new_result[i]);
    // }
    // debug(0, "\n");
    
    alnat_free(result);
    result = new_result;
    alnat_free(one);

    debug(-1, "/_alnat_make_complement\n");
    return result;
}

// Strips an alnat from useless zeroes
void _alnat_strip(Alnat_t* alnat) {
    struct AlnatMarcher m = _alnat_make_marcher(*alnat);
    size_t first_useless_zero = 0;

    // Find the start of the useless zeroes
    do {
        if (_alnat_get_curr_digit(m) != 0) {
            first_useless_zero = _alnat_get_marcher_pos(m) + 1;
        }
    } while (_alnat_move_forward(&m));

    // Strip them
    if (first_useless_zero > 0 &&
        first_useless_zero <= _alnat_get_marcher_pos(m)) {
        // printf("Resizing to %d\n", last_non_zero + 1);
        Alnat_t new_alnat = (Alnat_t)allocate_mem("_alnat_strip", *alnat,
            first_useless_zero);
        if (new_alnat == NULL) {
            error("_alnat_strip: Error while reallocating to %d\n",
                first_useless_zero);
            return;
        }
        *alnat = new_alnat;
        (*alnat)[first_useless_zero - 1] &= ~ALNAT_MAX;
    }
}

// Return 1 if first alnat is greater, -1 if second is greater, 0 if equal
int8_t _alnat_compare(Alnat_t a1, Alnat_t a2) {
    debug(1, "_alnat_compare\n");
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
                debug(-1, "/_alnat_compare\n");
                return a1_gt_a2;
            } else {
                debug(-1, "/_alnat_compare\n");
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
    debug(-1, "/_alnat_compare\n");
    return a1_gt_a2;
}

// Reduce by the GCD of two alnats using the Euclidean method
// https://en.wikipedia.org/wiki/Euclidean_algorithm
Alnat_t alnat_gcd(Alnat_t a1, Alnat_t a2) {
    debug(1, "alnat_gcd\n");

    if (alnat_is_null(a1) || alnat_is_null(a2)) {
        debug(1, "/alnat_gcd\n");
        return alnat_make_single_digit(0);
    }
    
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
    debug(-1, "/alnat_gcd\n");
    return greater;
}

void alnat_print(Alnat_t alnat) {
    char* buf = alnat_to_string(alnat);
    printf("%s", buf);
    free_mem("alnat_print", buf);
}

void alnat_print_bytes(Alnat_t alnat) {
    struct AlnatMarcher m = _alnat_make_marcher(alnat);
    do {
        printf("%d", _alnat_get_curr_digit(m));
        if (!_alnat_is_end(m)) {
            printf(" ");
        }
    } while (_alnat_move_forward(&m));
}

