#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include "global.h"

#include <stdlib.h>

#ifdef DEBUG_PRINTS
#include <stdio.h>
#endif

struct _LoggerEntry {
    void* ptr;
    size_t size;
    struct _LoggerEntry* next;
};

struct _Logger {
    struct _LoggerEntry *entries;
    int entry_count;
    size_t current;
    size_t peak;
    size_t total;
};

extern struct _Logger _logger;

void    init_logger         ();

void*   allocate_mem        (char* comment, void* block, size_t size);
void    free_mem            (char* comment, void* ptr);

void    _add_entry          (char* comment, size_t, void*);
void    _del_entry          (char* comment, void*);

void    show_logger_entries (struct _Logger);

#endif