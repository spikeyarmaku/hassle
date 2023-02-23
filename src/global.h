#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "config.h"

#ifdef DEBUG
#include <stdarg.h>
#include <stdio.h>
#endif

int error(const char* s, ...);

#endif