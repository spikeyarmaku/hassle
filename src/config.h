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

// --- KNOBS ---

// How many bytes are allocated if a string needs to be expanded
#define STRING_BUFFER_SIZE 1024

// How many bytes are allocated if an alnat needs to be expanded
#define ALNAT_BUFFER_SIZE 8

// How many bytes are allocated if an expr needs to be expanded
#define EXPR_BUFFER_SIZE 8

// If a dict has to be grown, enough space is allocated for this many elements
#define DICT_BUFFER_SIZE 8

// If a stack has to be grown, enough space is allocated for this many elements
#define STACK_BUFFER_SIZE 8

#endif