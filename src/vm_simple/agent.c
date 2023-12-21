#include "agent.h"

struct Agent* agent_make(uint8_t type) {
    struct Agent* agent =
        allocate_mem("agent_make", NULL, sizeof(struct Agent));
    agent->type = type;
    for (uint8_t i = 0; i < MAX_PORT_NUM; i++) {
        agent->ports[i] = NULL;
    }
    return agent;
}

struct Agent* agent_make_name() {
    return agent_make(ID_NAME);
}

void agent_free(struct Agent* agent) {
    if (agent == NULL) {
        return;
    }
    
    free_mem("agent_free", agent);
}

void agent_free_rec(struct Agent* agent) {
    if (agent == NULL) {
        return;
    }

    for (uint8_t i = 0; i < MAX_PORT_NUM; i++) {
        agent_free_rec(agent->ports[i]);
    }
    agent_free(agent);
}
