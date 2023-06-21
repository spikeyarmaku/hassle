#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "config.h"

#ifdef DEBUG_PRINTS
#include <stdarg.h>
#include <stdio.h>
#endif

#define WORD size_t

#define BOOL uint8_t
#define FALSE 0
#define TRUE 1

#define DEPTH uint32_t

enum ErrorCode {Success, Error};
typedef enum ErrorCode ErrorCode_t;

void    error           (const char* s, ...);
void    debug           (const char* s, ...);
void    debug_start     (const char* s, ...);
void    debug_end       (const char* s, ...);
void    debug_on        ();
void    debug_off       ();

WORD    bytes_to_word   (uint8_t*, uint8_t);
void    word_to_bytes   (WORD, uint8_t*, uint8_t);
char*   str_cpy         (char*);

#endif