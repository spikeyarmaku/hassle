#ifndef _AGENT_H_
#define _AGENT_H_

#include "global.h"

#include "pool.h"

typedef struct Agent Agent;

struct Agent*   agent_make      (uint8_t, uint8_t);
void            agent_free      (struct Agent*);
void            agent_set_port  (struct Agent*, uint8_t, struct Agent*);
void            agent_set_type  (struct Agent*, uint8_t);

#endif