#include "memory.h"

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

struct _Logger _logger;

void init_logger() {
    _logger.entries = NULL;
    _logger.entry_count = 0;
    _logger.current = 0;
    _logger.peak = 0;
    _logger.total = 0;
}

void    _add_entry          (char*, size_t, void*);
void    _del_entry          (char*, void*);

void* allocate_mem(char* comment, void* ptr, size_t size) {
    void* result = NULL;
    if (ptr == NULL) {
        result = malloc(size);
        #ifdef MEMORY_DIAGNOSTIC
        _add_entry(comment, size, result);
        #endif
    } else {
        #ifdef MEMORY_DIAGNOSTIC
        _del_entry(comment, ptr);
        #endif
        result = realloc(ptr, size);
        #ifdef MEMORY_DIAGNOSTIC
        _add_entry(comment, size, result);
        #endif
    }
    assert(result != NULL);
    return result;
}

void free_mem(char* comment, void* ptr) {
    #ifdef MEMORY_DIAGNOSTIC
    _del_entry(comment, ptr);
    #endif
    #ifdef SECURE_DESTRUCTION
    // TODO figure out how to find out the size of the block to be overwritten
    #endif
    free(ptr);
}

void _add_entry(char* comment, size_t size, void* ptr) {
    if (ptr == NULL) {
        debug("New ptr is NULL\n");
        return;
    }
    // Create new entry
    struct _LoggerEntry* new_entry =
        (struct _LoggerEntry*)malloc(sizeof(struct _LoggerEntry));
    new_entry->ptr = ptr;
    new_entry->size = size;
    
    // March through the list until we find the first elem which is bigger
    struct _LoggerEntry* entry = _logger.entries;
    if (entry == NULL) {
        _logger.entries = new_entry;
        new_entry->next = NULL;
    } else {
        while (entry->next != NULL && entry->next->ptr < ptr) {
            entry = entry->next;
        }
        if (entry->ptr == ptr) {
            debug(
                "=== PANIC! Adding an already existing elem: %llu (%s)===\n",
                ptr, comment);
        } else {
            new_entry->next = entry->next;
            entry->next = new_entry;
        }
    }
    
    // Update entry count and peak size
    _logger.entry_count++;
    _logger.current += size;
    _logger.total += size;
    if (_logger.peak < _logger.current) {
        _logger.peak = _logger.current;
    }
    
    debug(
        "curr: %llu | peak: %llu | total: %llu | count: %d"
        " | Allocating %llu bytes at %llu (%s)\n",
        _logger.current, _logger.peak, _logger.total, _logger.entry_count, size,
        (size_t)ptr, comment);
}

void _del_entry(char* comment, void* ptr) {
    if (ptr == NULL) {
        debug("Ptr to delete is NULL\n");
        return;
    }
    // March through the list until we find the elem
    if (_logger.entries == NULL) {
        debug("=== PANIC! Deleting from empty list: %llu (%s)===\n", ptr,
            comment);
        return;
    }
    
    struct _LoggerEntry* entry = _logger.entries;
    if (entry->ptr == ptr) {
        _logger.entry_count--;
        _logger.current -= entry->size;
        debug(
            "curr: %llu | peak: %llu | total: %llu | count: %d"
            " | Deleting %llu bytes at %llu (%s)\n",
            _logger.current, _logger.peak, _logger.total, _logger.entry_count,
            _logger.entries->size, (size_t)ptr, comment);
        if (entry->next == NULL) {
            _logger.entries = NULL;
        } else {
            _logger.entries = entry->next;
        }
        free(entry);
        return;
    }

    while (entry->next != NULL && entry->next->ptr != ptr) {
        entry = entry->next;
    }

    if (entry->next == NULL) {
        debug("=== PANIC! Deleting non-existing elem: %llu (%s)===\n", ptr,
            comment);
    }
    
    // Free elem
    struct _LoggerEntry* to_delete = entry->next;
    _logger.entry_count--;
    _logger.current -= to_delete->size;
    entry->next = entry->next->next;
    debug(
        "curr: %llu | peak: %llu | total: %llu | count: %d"
        " | Deleting %llu bytes at %llu (%s)\n",
        _logger.current, _logger.peak, _logger.total, _logger.entry_count,
        to_delete->size, (size_t)ptr, comment);
    free(to_delete);
}

void show_logger_entries() {
    debug("Allocated memory:\n---------------\n");
    struct _LoggerEntry* e = _logger.entries;
    size_t i = 0;
    while (e != NULL) {
        debug("%llu. %llu bytes at %llu\n", i, e->size, (size_t)e->ptr);
        e = e->next;
        i++;
    }
}
