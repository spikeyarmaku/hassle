#include "agent.h"

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

void agent_free(struct Agent* agent) {
    assert(agent != NULL);

    if (agent->ports != NULL) {
        free_mem("agent_free/ports", agent->ports);
    }
    free_mem("agent", agent);
}

void agent_set_port(struct Agent* agent, uint8_t port_num, struct Agent* dst) {
    agent->ports[port_num] = dst;
}

void agent_set_type(struct Agent* agent, uint8_t type) {
    agent->type = type;
}
