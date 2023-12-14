#include "pool.h"

struct PtrPool {
    size_t capacity;
    size_t count;
    void** elems;
};

struct BytePool {
    size_t capacity;
    size_t count;
    size_t elem_size;
    uint8_t* data;
};

// --- PtrPool ---

struct PtrPool* ptrpool_make() {
    struct PtrPool* pool =
        allocate_mem("ptrpool_make", NULL, sizeof(struct PtrPool));
    pool->capacity = 0;
    pool->count = 0;
    pool->elems = NULL;
    return pool;
}

struct PtrPool* ptrpool_make_fixed(size_t count) {
    struct PtrPool* pool =
        allocate_mem("ptrpool_make", NULL, sizeof(struct PtrPool));
    pool->capacity = 0;
    pool->count = 0;
    pool->elems = allocate_mem("fixedptrpool_make/elems", NULL,
        sizeof(void*) * count);
    return pool;
}

void ptrpool_add(struct PtrPool* pool, void* elem) {
    while (pool->count + 1 > pool->capacity) {
        pool->capacity = pool->capacity == 0 ?
            BUFFER_SIZE : pool->capacity * BUFFER_FACTOR;
        pool->elems = allocate_mem("ptrpool_add_elem", pool->elems,
            pool->capacity * sizeof(void*));
    }

    ptrpool_add_unsafe(pool, elem);
}

void ptrpool_add_unsafe(struct PtrPool* pool, void* elem) {
    pool->elems[pool->count] = elem;
    pool->count++;
}

size_t ptrpool_get_count(struct PtrPool* pool) {
    return pool->count;
}

void ptrpool_set(struct PtrPool* pool, size_t index, void* elem) {
    if (pool->capacity > index) {
        pool->elems[index] = elem;
    }
}

void* ptrpool_get(struct PtrPool* pool, size_t index) {
    if (pool->capacity > index) {
        return pool->elems[index];
    } else {
        return NULL;
    }
}

// --- BytePool ---

struct BytePool* bytepool_make(size_t elem_size) {
    struct BytePool* pool =
        allocate_mem("bytepool_make", NULL, sizeof(struct BytePool));
    pool->capacity = 0;
    pool->count = 0;
    pool->elem_size = elem_size;
    pool->data = NULL;
    return pool;
}

void bytepool_add_elems(struct BytePool* pool, size_t elem_count,
    uint8_t* data)
{
    size_t byte_capacity = pool->capacity * pool->elem_size;
    size_t bytes_used = pool->count * pool->elem_size;
    while (bytes_used + (elem_count * pool->elem_size) > byte_capacity) {
        pool->capacity = pool->capacity == 0 ?
            BUFFER_SIZE : pool->capacity * BUFFER_FACTOR;
    }
    if (byte_capacity > pool->capacity * pool->elem_size) {
        pool->data = allocate_mem("bytepool_add_elems", pool->data,
            pool->capacity * pool->elem_size);
    }

    memcpy(pool->data + pool->count * pool->elem_size, data,
        elem_count * pool->elem_size);
    pool->count += elem_count;
}

void bytepool_add_elem(struct BytePool* pool, uint8_t* data) {
    bytepool_add_elems(pool, 1, data);
}

uint8_t bytepool_get_byte(struct BytePool* pool, size_t index) {
    return pool->data[index];
}

