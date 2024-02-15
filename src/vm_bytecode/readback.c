#include "readback.h"

// TODO make it work with inbetween steps, and make it return a tree
struct Program* readback_agent(struct Agent* agent) {
    if (agent_get_type(agent) == ID_NAME) {
        struct Agent* port_0 = agent_get_port(agent, 0);
        if (port_0 == NULL) {
            // Agent is name - this should only happen if the current agent is
            // the interface
            return NULL;
        } else {
            // Agent is indirection
            return readback_agent(port_0);
        }
    } else {
        switch (agent_get_type(agent)) {
            case ID_K: {
                return program_make_leaf();
            }
            case ID_S: {
                return program_make_stem(readback_agent(agent_get_port(
                    agent, 0)));
            }
            case ID_F: {
                return program_make_fork(
                    readback_agent(agent_get_port(agent, 0)),
                    readback_agent(agent_get_port(agent, 1)));
            }
            default: {
                printf("readback_agent: invalid agent type %d\n",
                    agent_get_type(agent));
                return NULL;
            }
        }
    }
    // return NULL;
}

struct Tree* readback(struct VM* vm) {
    return tree_make_program(readback_agent(vm_get_interface(vm)));
}

// TODO figure out a better way - this is from the old kin-using variation
struct VMData vm_serialize(struct VM* vm, uint8_t word_size) {
    assert(word_size <= sizeof(size_t));
    printf("Serializing VM\n");
    Serializer_t* serializer = serializer_init(word_size);

    tree_serialize(serializer, readback(vm));
    // serializer_write_word(serializer, 0);
    
    struct VMData vm_data;
    vm_data.data = serializer_get_data(serializer);
    vm_data.data_size = serializer_get_data_size(serializer);
    serializer_free_toplevel(serializer);
    return vm_data;
}