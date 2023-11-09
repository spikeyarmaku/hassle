#ifndef _CONFIG_H_
#define _CONFIG_H_

// --- SWITCHES ---

// Comment this out to disable unicode support. Useful for low-memory
// environments
// TODO implement unicode support
// # define UNICODE

// Comment this out to disable logging memory usage
// #define MEMORY_DIAGNOSTIC

// Comment this out to disable debug prints
// #define DEBUG_PRINTS

// --- KNOBS ---

// TODO rewrite memory allocations to multiply instead of keep adding memory

#define BUFFER_SIZE 8
#define BUFFER_SIZE_MULTIPLY_FACTOR 2

// How many bytes are allocated if a string needs to be expanded
#define STRING_BUFFER_SIZE 1024

// How many bytes are allocated if an alnat needs to be expanded
#define ALNAT_BUFFER_SIZE BUFFER_SIZE

// How many bytes are allocated if an expr needs to be expanded
#define EXPR_BUFFER_SIZE BUFFER_SIZE

#define STACK_BUFFER_SIZE BUFFER_SIZE

#endif