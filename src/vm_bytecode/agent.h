#ifndef _AGENT_H_
#define _AGENT_H_

#include "global.h"

typedef struct Agent Agent;

struct Agent*   agent_make      (uint8_t, uint8_t);
struct Agent*   agent_make_name ();
// struct Agent*   agent_make_ind  ();
uint8_t         agent_get_type  (struct Agent*);
void            agent_free      (struct Agent*);
void            agent_set_port  (struct Agent*, uint8_t, struct Agent*);
struct Agent*   agent_get_port  (struct Agent*, uint8_t);
void            agent_set_type  (struct Agent*, uint8_t);

void            agent_print     (struct Agent*);

extern const char* AgentNameTable[];

#endif