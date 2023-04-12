#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include "global.h"

#include <stdlib.h>

#ifdef DEBUG_PRINTS
#include <stdio.h>
#endif

void    init_logger         ();

void*   allocate_mem        (char*, void*, size_t);
void    free_mem            (char*, void*);

void    show_logger_entries ();

#endif