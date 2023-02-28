#include "memory.h"

void init_logger() {
    #ifdef MEMORY_DIAGNOSTIC
    _logger.entries = NULL;
    _logger.entry_count = 0;
    _logger.current = 0;
    _logger.peak = 0;
    _logger.total = 0;
    #endif
}

int get_index_for_ptr(void* ptr) {
    printf("get_index\n");
    int index = 0;
    struct LoggerEntry* entry = _logger.entries;
    while (entry != NULL && entry->ptr != ptr) {
        entry = entry->next;
        if (entry == NULL) {
            return -1;
        }
        index++;
    }

    return index;
}

void* alloc_mem(size_t size) {
    void* ptr = malloc(size);
    #ifdef MEMORY_DIAGNOSTIC
    add_entry(size, ptr);
    #endif
    return ptr;
}

void* realloc_mem(void* block, size_t size) {
    void* ptr = realloc(block, size);
    #ifdef MEMORY_DIAGNOSTIC
    del_entry(block);
    add_entry(size, ptr);
    #endif
    return ptr;
}

void free_mem(void* ptr) {
    free(ptr);
    #ifdef MEMORY_DIAGNOSTIC
    del_entry(ptr);
    #endif
}

void add_entry(size_t size, void* ptr) {
    if (ptr == NULL) {
        #ifdef DEBUG
        printf("New ptr is NULL");
        #endif
        return;
    }
    // Create new entry
    struct LoggerEntry* new_entry =
        (struct LoggerEntry*)malloc(sizeof(struct LoggerEntry));
    new_entry->ptr = ptr;
    new_entry->size = size;
    
    // March through the list until we find the first elem which is bigger
    struct LoggerEntry* entry = _logger.entries;
    if (entry == NULL) {
        _logger.entries = new_entry;
        new_entry->next = NULL;
    } else {
        while (entry->next != NULL && entry->next->ptr < ptr) {
            entry = entry->next;
        }
        if (entry->ptr == ptr) {
            #ifdef DEBUG
            printf("=== PANIC! Adding an already existing elem: %llu ===\n", ptr);
            #endif
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
    
    #ifdef DEBUG
    printf("curr: %llu | peak: %llu | total: %d | count: %d | Allocating %d bytes at %llu\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, size, ptr);
    #endif
}

void del_entry(void* ptr) {
    if (ptr == NULL) {
        #ifdef DEBUG
        printf("Ptr to delete is NULL");
        #endif
        return;
    }
    // March through the list until we find the elem
    if (_logger.entries == NULL) {
        #ifdef DEBUG
        printf("=== PANIC! Deleting from empty list: %llu ===\n", ptr);
        #endif
        return;
    }
    
    struct LoggerEntry* entry = _logger.entries;
    if (entry->ptr == ptr) {
        _logger.entry_count--;
        _logger.current -= entry->size;
        #ifdef DEBUG
        printf("curr: %llu | peak: %llu | total: %d | count: %d | Deleting %d bytes at %llu\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, _logger.entries->size, ptr);
        #endif
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
        #ifdef DEBUG
        printf("=== PANIC! Deleting non-existing elem: %llu ===\n", ptr);
        #endif
    }
    
    // Free elem
    struct LoggerEntry* to_delete = entry->next;
    _logger.entry_count--;
    _logger.current -= to_delete->size;
    entry->next = entry->next->next;
    #ifdef DEBUG
    printf("curr: %llu | peak: %llu | total: %d | count: %d | Deleting %d bytes at %llu\n", _logger.current, _logger.peak, _logger.total, _logger.entry_count, to_delete->size, ptr);
    #endif
    free(to_delete);
}

