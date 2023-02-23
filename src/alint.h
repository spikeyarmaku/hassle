#ifndef _ALINT_H_
#define _ALINT_H_

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "global.h"

// Important: if the << operator is used, it must be put in parenthesis
#define ALINT_MAX 128

// Arbitrary Length INTeger, least significant digit first
// Each byte contains a digit in base 128, and each byte has its highest bit set
// to one, except for the last byte, which has its highest bit set to 0.
typedef uint8_t* Alint;

typedef uint8_t ErrorCode;

struct AlintBuilder {
    Alint _ptr;
    size_t _size; // The size of the allocated array
    size_t _next; // Always points to the next empty address
};

struct AlintMarcher {
    Alint _ptr;
    size_t _counter;
    uint8_t _finished;
};

void debug_print_alint_builder(struct AlintBuilder);

struct AlintBuilder create_alint_builder();
Alint get_alint(struct AlintBuilder);
// Add a new block to the alint's memory
ErrorCode expand_alint(struct AlintBuilder*);
// Add a new digit to alint
ErrorCode add_digit_to_alint(uint8_t, struct AlintBuilder*);
// Free up unused memory and set last byte to 0
ErrorCode finalize_alint(struct AlintBuilder*);

struct AlintMarcher create_alint_marcher(Alint);
uint8_t get_next_alint_digit(struct AlintMarcher*);
uint8_t is_end_of_alint(struct AlintMarcher);
size_t get_marcher_position(struct AlintMarcher);
void rewind_marcher(struct AlintMarcher*);

ErrorCode unsafe_get_digit_alint(size_t, Alint);
ErrorCode unsafe_is_last_digit_alint(size_t, Alint);
void unsafe_mark_digit_alint(size_t, uint8_t, Alint);

Alint string_to_alint(char*);
char* debug_print_alint(Alint);
char* alint_to_string(Alint);
Alint destroy_alint(Alint);
Alint make_single_digit_alint(uint8_t);
uint8_t is_null_alint(Alint);
Alint copy_alint(Alint);

Alint add_alint(Alint, Alint);
Alint sub_alint(Alint, Alint, int8_t*);
Alint mul_alint(Alint, Alint);
Alint div_alint(Alint, Alint);

Alint make_complement_alint(Alint);
void strip_alint(Alint*);
int8_t compare_alint(Alint, Alint);
Alint gcd_alint(Alint, Alint);

#endif