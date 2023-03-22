#include "memory.h"

void init_logger() {
    _logger.entries = NULL;
    _logger.entry_count = 0;
    _logger.current = 0;
    _logger.peak = 0;
    _logger.total = 0;
}

void* allocate_mem(char* comment, void* ptr, size_t size) {
    if (ptr == NULL) {
        void* ptr = malloc(size);
        #ifdef MEMORY_DIAGNOSTIC
        _add_entry(comment, size, ptr);
        #endif
        return ptr;
    } else {
        #ifdef MEMORY_DIAGNOSTIC
        _del_entry(comment, ptr);
        #endif
        void* new_ptr = realloc(ptr, size);
        #ifdef MEMORY_DIAGNOSTIC
        _add_entry(comment, size, new_ptr);
        #endif
        return new_ptr;
    }
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
        debug(0, "New ptr is NULL");
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
            debug(0, "=== PANIC! Adding an already existing elem: %llu ===\n",
                ptr);
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
    
    debug(0, "curr: %llu | peak: %llu | total: %d | count: %d | Allocating %d bytes at %llu (%s)\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, size, ptr, comment);
}

void _del_entry(char* comment, void* ptr) {
    if (ptr == NULL) {
        debug(0, "Ptr to delete is NULL\n");
        return;
    }
    // March through the list until we find the elem
    if (_logger.entries == NULL) {
        debug(0, "=== PANIC! Deleting from empty list: %llu ===\n", ptr);
        return;
    }
    
    struct _LoggerEntry* entry = _logger.entries;
    if (entry->ptr == ptr) {
        _logger.entry_count--;
        _logger.current -= entry->size;
        debug(0, "curr: %llu | peak: %llu | total: %d | count: %d | Deleting %d bytes at %llu (%s)\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, _logger.entries->size, ptr, comment);
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
        debug(0, "=== PANIC! Deleting non-existing elem: %llu ===\n", ptr);
    }
    
    // Free elem
    struct _LoggerEntry* to_delete = entry->next;
    _logger.entry_count--;
    _logger.current -= to_delete->size;
    entry->next = entry->next->next;
    debug(0, "curr: %llu | peak: %llu | total: %d | count: %d | Deleting %d bytes at %llu (%s)\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, to_delete->size, ptr, comment);
    free(to_delete);
}

void show_logger_entries(struct _Logger l) {
    debug(0, "Allocated memory:\n---------------\n");
    struct _LoggerEntry* e = l.entries;
    size_t i = 0;
    while (e != NULL) {
        debug(0, "%llu. %llu bytes at %llu\n", i, e->size, (size_t)e->ptr);
        e = e->next;
        i++;
    }
}

