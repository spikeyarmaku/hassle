#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "config.h"
#include "global.h"

#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

struct LoggerEntry {
    void* ptr;
    size_t size;
    struct LoggerEntry* next;
};

struct Logger {
    struct LoggerEntry *entries;
    int entry_count;
    size_t current;
    size_t peak;
    size_t total;
};

extern struct Logger _logger;

void init_logger();
int get_index_for_ptr(void* ptr);

void* alloc_mem(size_t size);
void* realloc_mem(void* block, size_t size);
void free_mem(void* ptr);

void add_entry(size_t, void*);
void del_entry(void*);
size_t count_allocated_bytes();

#endif