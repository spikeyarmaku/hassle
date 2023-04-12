#ifndef _CONFIG_H_
#define _CONFIG_H_

// --- SWITCHES ---

// Comment this out to disable unicode support. Useful for low-memory
// environments
// TODO implement unicode support
// # define UNICODE

// Comment this out to disable the repl
#define REPL_ENABLED

// Comment this out to disable logging memory usage
// #define MEMORY_DIAGNOSTIC

// Comment this out to disable debug prints
// #define DEBUG_PRINTS

// Comment this out to disable memoization. Useful in low-memory environments
// #define MEMOIZE_SUB_EXPRS

// Comment this out to disable overwriting memory with random data before
// freeing it up. It's useful for making sure sensitive data resides in the
// memory only for as long as necessary, at the cost of some performance hit.
// TODO do it in memory.h
// #define SECURE_DESTRUCTION

// --- KNOBS ---

// The amount of debug prints, if DEBUG_PRINTS is defined
#define DEBUG_LEVEL 200

// How many bytes are allocated if a string needs to be expanded
#define STRING_BUFFER_SIZE 1024

// How many bytes are allocated if an alnat needs to be expanded
#define ALNAT_BUFFER_SIZE 8

// How many bytes are allocated if an expr needs to be expanded
#define EXPR_BUFFER_SIZE 8

// If a dict ha to be grown, enough space is allocated for this many elements
#define DICT_BUFFER_SIZE 8

#endif