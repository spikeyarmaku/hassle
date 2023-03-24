// TODO Mark what is public and what is private

#ifndef _ALNAT_H_
#define _ALNAT_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "global.h"

#include "memory.h"

// Important: if the << operator is used, it must be put in parenthesis
#define ALNAT_MAX 128

// Arbitrary Length NATural, least significant digit first
// Each byte contains a digit in base 128, and each byte has its highest bit set
// to one, except for the last byte, which has its highest bit set to 0.
typedef uint8_t* Alnat_t;

struct AlnatBuilder {
    Alnat_t ptr;
    size_t size; // The size of the allocated array
    size_t next; // Always points to the next empty address
};

struct AlnatMarcher {
    Alnat_t ptr;
    size_t counter;
};

Alnat_t             string_to_alnat             (char*);
char*               alnat_to_string             (Alnat_t);
void                alnat_free                  (Alnat_t);
Alnat_t             alnat_make_single_digit     (uint8_t);
BOOL                alnat_is_null               (Alnat_t);
BOOL                alnat_is_equal              (Alnat_t, Alnat_t);

Alnat_t             alnat_add                   (Alnat_t, Alnat_t);
Alnat_t             alnat_sub                   (Alnat_t, Alnat_t, int8_t*);
Alnat_t             alnat_mul                   (Alnat_t, Alnat_t);
Alnat_t             alnat_div                   (Alnat_t, Alnat_t);

Alnat_t             alnat_gcd                   (Alnat_t, Alnat_t);

void                alnat_print                 (Alnat_t);
void                alnat_print_bytes           (Alnat_t);

// --- Private methods ---

struct AlnatBuilder _alnat_make_builder         ();
// Add a new block to the alnat's memory
enum ErrorCode      _alnat_expand               (struct AlnatBuilder*);
// Add a new digit to alnat
enum ErrorCode      _alnat_add_digit            (uint8_t, struct AlnatBuilder*);
// Free up unused memory and set last byte to 0
enum ErrorCode      _alnat_finalize             (struct AlnatBuilder*);

struct AlnatMarcher _alnat_make_marcher         (Alnat_t);
uint8_t             _alnat_get_curr_digit       (struct AlnatMarcher);
BOOL                _alnat_is_start             (struct AlnatMarcher);
BOOL                _alnat_is_end               (struct AlnatMarcher);
BOOL                _alnat_move_forward         (struct AlnatMarcher*);
BOOL                _alnat_move_backward        (struct AlnatMarcher*);
uint8_t             _alnat_get_next_digit       (struct AlnatMarcher*);
uint8_t             _alnat_get_prev_digit       (struct AlnatMarcher*);
size_t              _alnat_get_marcher_pos      (struct AlnatMarcher);
void                _alnat_rewind_marcher       (struct AlnatMarcher*);

uint8_t             _alnat_unsafe_get_digit     (size_t, Alnat_t);
BOOL                _alnat_unsafe_is_last_digit (size_t, Alnat_t);
void                _alnat_unsafe_mark_digit    (size_t, uint8_t, Alnat_t);

void                _alnat_str_double           (char*);
void                _alnat_str_add              (char*, char*);
Alnat_t             _alnat_copy                 (Alnat_t);

Alnat_t             _alnat_make_complement      (Alnat_t);
void                _alnat_strip                (Alnat_t*);
int8_t              _alnat_compare              (Alnat_t, Alnat_t);

#endif