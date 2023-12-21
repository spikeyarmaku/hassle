#ifndef _AGENT_H_
#define _AGENT_H_

#include <stdint.h>

#include "memory.h"

#define MAX_PORT_NUM 3

#define ID_NAME         0
#define ID_K            1
#define ID_S            2
#define ID_F            3
#define ID_APP          4
#define ID_APP1         5
#define MAX_AGENT_ID    6

char Symbols[MAX_AGENT_ID] = {0, 'K', 'S', 'F', '@', '#'};
uint8_t Arities[MAX_AGENT_ID] = {1, 0, 1, 2, 2, 3};

struct Agent {
    uint8_t type;
    struct Agent* ports[MAX_PORT_NUM];
};

struct Agent*   agent_make      (uint8_t);
struct Agent*   agent_make_name ();
void            agent_free      (struct Agent*);
void            agent_free_rec  (struct Agent*);

#endif