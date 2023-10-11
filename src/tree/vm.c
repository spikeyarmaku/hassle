#include "vm.h"

#include "tree/eval.h"

struct VM {
    struct Term* term;
};

struct VM* vm_init() {
    struct VM* vm = allocate_mem("vm_init", NULL, sizeof(struct VM));
    vm->term = NULL;
    return vm;
}

void vm_set_term(struct VM* vm, struct Term* term) {
    vm->term = term;
}

enum EvalState vm_step(struct VM* vm) {
    vm->term = eval_step(vm->term);
    return eval_done(vm->term) == TRUE ? EvalFinished : EvalRunning;
}

struct Term* vm_run(struct VM* vm) {
    return eval(vm->term);
}

struct VMData vm_serialize(struct VM* vm, uint8_t word_size) {
    assert(word_size <= sizeof(size_t));
    Serializer_t* serializer = serializer_init(word_size);
    term_serialize(serializer, vm->term);
    struct VMData vm_data;
    vm_data.data = serializer_get_data(serializer);
    vm_data.data_size = serializer_get_data_size(serializer);
    return vm_data;
}

void vm_reset(struct VM* vm) {
    vm->term = NULL;
}

void vm_free(struct VM* vm) {
    term_free(vm->term);
    free_mem("vm_free", vm);
}
