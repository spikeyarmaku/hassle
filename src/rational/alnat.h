// TODO Mark what is public and what is private

#ifndef _ALNAT_H_
#define _ALNAT_H_

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "global.h"

#include "memory.h"

// Important: if the << operator is used, it must be put in parenthesis
#define ALNAT_MAX 128

// Arbitrary Length NATural, least significant digit first
// Each byte contains a digit in base 128, and each byte has its highest bit set
// to one, except for the last byte, which has its highest bit set to 0.
typedef uint8_t* Alnat;

struct AlnatBuilder {
    Alnat ptr;
    size_t size; // The size of the allocated array
    size_t next; // Always points to the next empty address
};

struct AlnatMarcher {
    Alnat ptr;
    size_t counter;
    uint8_t finished;
};

struct AlnatBuilder make_alnat_builder      ();
// Add a new block to the alnat's memory
ErrorCode           expand_alnat            (struct AlnatBuilder*);
// Add a new digit to alnat
ErrorCode           add_digit_to_alnat      (uint8_t, struct AlnatBuilder*);
// Free up unused memory and set last byte to 0
ErrorCode           finalize_alnat          (struct AlnatBuilder*);

struct AlnatMarcher make_alnat_marcher      (Alnat);
uint8_t             get_next_alnat_digit    (struct AlnatMarcher*);
BOOL                is_end_of_alnat         (struct AlnatMarcher);
size_t              get_marcher_position    (struct AlnatMarcher);
void                rewind_marcher          (struct AlnatMarcher*);

ErrorCode           unsafe_get_digit_alnat      (size_t, Alnat);
ErrorCode           unsafe_is_last_digit_alnat  (size_t, Alnat);
void                unsafe_mark_digit_alnat     (size_t, uint8_t, Alnat);

Alnat               string_to_alnat         (char*);
char*               debug_print_alnat       (Alnat);
char*               alnat_to_string         (Alnat);
void                free_alnat              (Alnat);
Alnat               make_single_digit_alnat (uint8_t);
BOOL                is_null_alnat           (Alnat);
Alnat               copy_alnat              (Alnat);

Alnat               add_alnat               (Alnat, Alnat);
Alnat               sub_alnat               (Alnat, Alnat, int8_t*);
Alnat               mul_alnat               (Alnat, Alnat);
Alnat               div_alnat               (Alnat, Alnat);

Alnat               make_complement_alnat   (Alnat);
void                strip_alnat             (Alnat*);
int8_t              compare_alnat           (Alnat, Alnat);
Alnat               gcd_alnat               (Alnat, Alnat);

#endif