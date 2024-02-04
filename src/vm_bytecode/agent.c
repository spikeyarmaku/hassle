#include "agent.h"

#include <stdio.h>

struct Agent {
    uint8_t type;
    struct Agent* ports[MAX_AUX_PORT_NUM];
};

struct Agent* agent_make(uint8_t type, uint8_t arity) {
    struct Agent* agent =
        allocate_mem("agent_make", NULL, sizeof(struct Agent));
    agent->type = type;
    for (uint8_t i = 0; i < MAX_AUX_PORT_NUM; i++) {
        agent->ports[i] = NULL;
    }
    return agent;
}

// Name and indirection nodes are made the same way. The difference is that name
// nodes have their port unconnected.
struct Agent* agent_make_name() {
    return agent_make(ID_NAME, 1);
}

uint8_t agent_get_type(struct Agent* agent) {
    return agent->type;
}

void agent_free(struct Agent* agent) {
    assert(agent != NULL);

    free_mem("agent", agent);
}

void agent_set_port(struct Agent* agent, uint8_t port_num, struct Agent* dst) {
    agent->ports[port_num] = dst;
}

struct Agent* agent_get_port(struct Agent* agent, uint8_t port_num) {
    return agent->ports[port_num];
}

void agent_set_type(struct Agent* agent, uint8_t type) {
    agent->type = type;
}

void agent_print(struct Agent* agent) {
    if (agent->type == ID_NAME) {
        if (agent->ports[0] == NULL) {
            printf(" <%llx>", (size_t)agent);
        } else {
            // printf(" [%llx] ", (size_t)agent);
            agent_print(agent->ports[0]);
        }
    } else {
        printf("%s", AgentNameTable[agent->type]);
        if (agent->ports[0] != NULL) {
            printf("(");
        }
        for (uint8_t i = 0; i < MAX_AUX_PORT_NUM; i++) {
            if (agent->ports[i] != NULL) {
                if (i > 0) {
                    printf(", ");
                }
                agent_print(agent->ports[i]);
            }
        }
        if (agent->ports[0] != NULL) {
            printf(")");
        }
    }
}

const char* AgentNameTable[MAX_AGENT_ID] = {
    "Name", "K", "S", "F", "Eraser", "Duplicator", "App", "Delta"};
