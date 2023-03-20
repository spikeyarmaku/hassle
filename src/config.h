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

// Comment this out to speed up the interpreter a bit at the expense of
// consuming somewhat more memory
// TODO implement bitpacking
// #define BITPACK_EXPR

// Comment this out to disable memoization. Useful in low-memory environments
// #define MEMOIZE_SUB_EXPRS

// --- KNOBS ---

// The amount of debug prints, if DEBUG_PRINTS is defined
#define DEBUG_LEVEL 2

// How many bytes are allocated if a string needs to be expanded
#define STRING_BUFFER_SIZE 1024

// How many bytes are allocated if an alnat needs to be expanded
#define ALNAT_BUFFER_SIZE 8

// How many bytes are allocated if an expr needs to be expanded
#define EXPR_BUFFER_SIZE 8

// The number of bytes in a symbol ID
#define SYMBOL_ID_BYTES 2

#endif