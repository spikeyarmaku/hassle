// TODO Mark what is public and what is private

#ifndef _ALNAT_H_
#define _ALNAT_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    uint8_t finished;
};

struct AlnatBuilder make_alnat_builder      ();
// Add a new block to the alnat's memory
enum ErrorCode      expand_alnat            (struct AlnatBuilder*);
// Add a new digit to alnat
enum ErrorCode      add_digit_to_alnat      (uint8_t, struct AlnatBuilder*);
// Free up unused memory and set last byte to 0
enum ErrorCode      finalize_alnat          (struct AlnatBuilder*);

struct AlnatMarcher make_alnat_marcher      (Alnat_t);
uint8_t             get_next_alnat_digit    (struct AlnatMarcher*);
BOOL                is_end_of_alnat         (struct AlnatMarcher);
size_t              get_marcher_position    (struct AlnatMarcher);
void                rewind_marcher          (struct AlnatMarcher*);

uint8_t             unsafe_get_digit_alnat      (size_t, Alnat_t);
uint8_t             unsafe_is_last_digit_alnat  (size_t, Alnat_t);
void                unsafe_mark_digit_alnat     (size_t, uint8_t, Alnat_t);

void                _double                 (char*);
void                _add                    (char*, char*);
Alnat_t             string_to_alnat         (char*);
char*               alnat_to_string         (Alnat_t);
void                free_alnat              (Alnat_t);
Alnat_t             make_single_digit_alnat (uint8_t);
BOOL                is_null_alnat           (Alnat_t);
BOOL                is_equal_alnat          (Alnat_t, Alnat_t);
Alnat_t             copy_alnat              (Alnat_t);

Alnat_t             add_alnat               (Alnat_t, Alnat_t);
Alnat_t             sub_alnat               (Alnat_t, Alnat_t, int8_t*);
Alnat_t             mul_alnat               (Alnat_t, Alnat_t);
Alnat_t             div_alnat               (Alnat_t, Alnat_t);

Alnat_t             make_complement_alnat   (Alnat_t);
void                strip_alnat             (Alnat_t*);
int8_t              compare_alnat           (Alnat_t, Alnat_t);
Alnat_t             gcd_alnat               (Alnat_t, Alnat_t);

#endif