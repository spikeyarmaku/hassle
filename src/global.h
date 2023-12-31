#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "config.h"

#ifdef DEBUG_PRINTS
#include <stdarg.h>
#include <stdio.h>
#endif

#define VM_WORD size_t

#define BOOL uint8_t
#define FALSE 0
#define TRUE 1

#define OUT

#define DEPTH uint32_t

// interaction net
#define MAX_AUX_PORT_NUM 3

#define ID_NAME         0 // Name
#define ID_K            1 // K, arity 0
#define ID_S            2 // S, arity 1
#define ID_F            3 // F, arity 2
#define ID_APP          4 // @, arity 2
#define ID_DELTA        5 // #, arity 3
#define MAX_AGENT_ID    6

// arities for bytecode ops
#define OP_MKAGENT  0   // reg type - Create an agent of `type` at `reg`
#define OP_MKNAME   1   // reg - Create a name at `reg`
#define OP_CONNECT  2   // reg0 port_num reg1 - Connect reg1 to reg0's `port`
#define OP_PUSH     3   // reg0 reg1 - Add reg0 and reg1 to the stack
#define OP_STORE    4   // reg g - store reg's content in the global reg g
#define OP_LOAD     5   // g reg - load global reg g's content into reg
#define OP_RETURN   6   // --

#define CODE_TABLE_SIZE ((MAX_AGENT_ID - 1) * MAX_AGENT_ID / 2)

#define VARIABLE_INDEX_START (MAX_AUX_PORT_NUM * 2)

// TODO limit it to absolutely necessary: MAX_AUX_NUM * 2 + max number of agents
// created in a rewrite rule
// #define MAX_REG_SIZE 64
#define MAX_REG_SIZE 13

enum ErrorCode {Success, Error};
typedef enum ErrorCode ErrorCode_t;

void    error           (const char* s, ...);
void    fatal           (const char* s, ...);
void    debug           (const char* s, ...);
void    debug_start     (const char* s, ...);
void    debug_end       (const char* s, ...);
void    debug_on        ();
void    debug_off       ();

VM_WORD bytes_to_word       (uint8_t*, uint8_t);
void    word_to_bytes       (VM_WORD, uint8_t*, uint8_t);
char*   str_cpy             (char*);
int     str_get_token_end   (const char*);
char*   str_get_substr      (const char*, int, BOOL);

#endif