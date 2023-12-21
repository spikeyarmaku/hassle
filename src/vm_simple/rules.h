#ifndef _RULES_H_
#define _RULES_H_

#include <stdint.h>

#include "vm_simple/agent.h"
#include "vm_simple/vm.h"

typedef void (*RuleFun)(struct VM*, struct Agent*, struct Agent*);
RuleFun RuleTable[MAX_AGENT_ID][MAX_AGENT_ID];

void    rule_table_init ();
void    missing_rule    (struct VM*, struct Agent*, struct Agent*);
void    rule_K_App      (struct VM*, struct Agent*, struct Agent*);
void    rule_S_App      (struct VM*, struct Agent*, struct Agent*);
void    rule_F_App      (struct VM*, struct Agent*, struct Agent*);
void    rule_K_App1     (struct VM*, struct Agent*, struct Agent*);
void    rule_S_App1     (struct VM*, struct Agent*, struct Agent*);
void    rule_F_App1     (struct VM*, struct Agent*, struct Agent*);
void    rule_K_F        (struct VM*, struct Agent*, struct Agent*);
void    rule_S_F        (struct VM*, struct Agent*, struct Agent*);
void    rule_F_F        (struct VM*, struct Agent*, struct Agent*);
void    rule_K_K        (struct VM*, struct Agent*, struct Agent*);
void    rule_K_S        (struct VM*, struct Agent*, struct Agent*);


#endif