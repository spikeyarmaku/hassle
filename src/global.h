#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>

#include "config.h"

#ifdef DEBUG_PRINTS
#include <stdarg.h>
#include <stdio.h>
#endif

#define BOOL uint8_t
#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define ERROR 1

#define DEPTH uint32_t

typedef uint8_t ErrorCode;

void error(const char* s, ...);
void debug(int, const char* s, ...);

#endif