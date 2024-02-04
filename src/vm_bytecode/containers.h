#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include <stdlib.h>

#include "global.h"
#include "agent.h"

struct Equation {
    struct Agent* agent0;
    struct Agent* agent1;
};

struct EqStack*     eq_stack_make           ();
void                eq_stack_push           (struct EqStack*, struct Equation);
struct Equation     eq_stack_pop            (struct EqStack*);
struct Equation     eq_stack_peek           (struct EqStack*, size_t);
void                eq_stack_free           (struct EqStack*);
size_t              eq_stack_size           (struct EqStack*);

struct AgentHeap*   agent_heap_make         ();
struct AgentHeap*   agent_heap_make_fixed   (size_t);
void                agent_heap_add          (struct AgentHeap*, struct Agent*);
void                agent_heap_add_unsafe   (struct AgentHeap*, struct Agent*);
size_t              agent_heap_get_count    (struct AgentHeap*);
void                agent_heap_set
    (struct AgentHeap*, size_t, struct Agent*);
struct Agent*       agent_heap_get          (struct AgentHeap*, size_t);
void                agent_heap_free         (struct AgentHeap*);

struct ByteArray*   byte_array_make         ();
void                byte_array_add_byte     (struct ByteArray*, uint8_t);
void                byte_array_add_word
    (struct ByteArray*, size_t, uint8_t);
uint8_t*            byte_array_get_bytes    (struct ByteArray*);

#endif