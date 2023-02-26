#include "memory.h"

void init_logger() {
    _logger.entry_count = 0;
    _logger.peak = 0;
}

int get_index_for_ptr(void* ptr) {
    for (int i = 0; i < _logger.entry_count; i++) {
        if (_logger.entries[i].ptr == ptr) {
            return i;
        }
    }

    return -1;
}

void* alloc_mem(size_t size) {
    void* ptr = malloc(size);
    // TODO handle NULL
    int index = get_index_for_ptr(ptr);
    if (index != -1) {
        _logger.entries[index].size = size;
    } else {
        struct LoggerEntry entry;
        entry.ptr = ptr;
        entry.size = size;
        _logger.entries[_logger.entry_count] = entry;
        _logger.entry_count++;
    }
    #ifdef DEBUG
    printf("%llu | %llu | Allocating %d bytes at %llu\n", count_allocated_bytes(), _logger.peak, size, ptr);
    #endif
    return ptr;
}

void* realloc_mem(void* block, size_t size) {
    void* ptr = realloc(block, size);
    // TODO handle NULL
    int index = get_index_for_ptr(block);
    if (index != -1) {
        _logger.entries[index].size = size;
        _logger.entries[index].ptr = ptr;
    } else {
        printf("=== PANIC! === Calling realloc on %llu which is not present in the table\n", block);
    }
    #ifdef DEBUG
    printf("%llu | %llu | Resizing %llu -> %llu to %d bytes\n", count_allocated_bytes(), _logger.peak, block, ptr, size);
    #endif
    return ptr;
}

void free_mem(void* ptr) {
    free(ptr);
    size_t size = 0;
    int index = get_index_for_ptr(ptr);
    if (index != -1) {
        size = _logger.entries[index].size;
        _logger.entries[index].size = 0;
    } else {
        printf("=== PANIC! === Calling free on %llu which is not present in the table\n", ptr);
    }
    #ifdef DEBUG
    printf("%llu | %llu | Freeing up %d bytes at %llu\n", count_allocated_bytes(), _logger.peak, size, ptr);
    #endif
}

size_t count_allocated_bytes() {
    size_t sum = 0;
    for (int i = 0; i < _logger.entry_count; i++) {
        sum += _logger.entries[i].size;
    }
    if (sum > _logger.peak) {
        _logger.peak = sum;
    }
    return sum;
}
