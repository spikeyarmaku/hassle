// Evaluation: first evaluate recursively the first three children, then add
// them to a node, and evaluate the resulting tree. In pseudocode:
// eval expr =
// let c0 = eval (get_child expr 0)
//     c1 = eval (get_child expr 1)
//     c2 = eval (get_child expr 2)
// in  eval (tree_make c0 c1 c2)
// For an example, see "doc/not_true_raw.gif" or "doc/not_true_annotated.gif"

#include "vm.h"

#include "stack.h"
#include "kin.h"

struct VM {
    struct Program* control;    // graft
    struct Stack* stack;        // host - Stack<Kin>
};

struct VM* vm_make() {
    struct VM* vm = allocate_mem("vm_make", NULL, sizeof(struct VM));
    vm->control = NULL;
    vm->stack = NULL;
    return vm;
}

void vm_populate(struct VM* vm, struct Tree* tree) {
    if (vm == NULL) {
        vm = vm_make();
    }
    if (vm->stack == NULL) {
        vm->stack = stack_make();
    }
    while (tree_get_type(tree) == TREE_TYPE_APPLY) {
        // struct Kin* kin = kin_make(FALSE, tree_copy(tree_get_apply(tree, 1)));
        // stack_push(vm->stack, kin);
        // struct Tree* temp = tree_copy(tree_get_apply(tree, 0));
        // tree_free(tree);
        // tree = temp;
        struct TreePair subtrees = tree_extract_subtrees(tree);
        struct Kin* kin = kin_make(FALSE, subtrees.tree1);
        stack_push(vm->stack, kin);
        tree = subtrees.tree0;
    }
    if (tree_get_type(tree) == TREE_TYPE_PROGRAM) {
        // vm->control = program_copy(tree_get_program(tree));
        // tree_free(tree);
        vm->control = tree_extract_program(tree);
    } else {
        fatal("vm_populate: invalid tree type %d\n", tree_get_type(tree));
    }
}

// TODO pay attention to evaluating operators
enum EvalState vm_step(struct VM* vm) {
    if (stack_is_empty(vm->stack)) {
        return EvalFinished;
    }

    struct Kin* kin = (struct Kin*)stack_pop(vm->stack);
    if (kin_is_parent(kin) == TRUE) {
        switch (tree_get_type(kin_get_tree(kin))) {
            case TREE_TYPE_PROGRAM: {
                // Swap control and top of stack
                // struct Program* new_control =
                //     program_copy(tree_get_program(kin_get_tree(kin)));
                // struct Kin* new_kin =
                //     kin_make(FALSE, tree_make_program(vm->control));
                // stack_push(vm->stack, new_kin);
                // vm->control = new_control;
                // kin_free(kin);
                // break;
                struct Program* new_control =
                    tree_extract_program(kin_extract_tree(kin));
                struct Kin* new_kin =
                    kin_make(FALSE, tree_make_program(vm->control));
                stack_push(vm->stack, new_kin);
                vm->control = new_control;
                break;
            }
            default: {
                fatal("vm_step: invalid tree type %d\n",
                    tree_get_type(kin_get_tree(kin)));
            }
        }
    } else {
        switch (tree_get_type(kin_get_tree(kin))) {
            case TREE_TYPE_PROGRAM: {
                struct Tree* new_tree =
                    tree_apply_programs(vm->control,
                        tree_get_program(kin_get_tree(kin)));
                vm_populate(vm, new_tree);
                break;
            }
            case TREE_TYPE_APPLY: {
                // struct Kin* new_kin =
                //     kin_make(TRUE, tree_make_program(vm->control));
                // stack_push(vm->stack, new_kin);
                // vm->control = NULL;
                // vm_populate(vm, tree_copy(kin_get_tree(kin)));
                // kin_free(kin);
                // break;
                struct Kin* new_kin =
                    kin_make(TRUE, tree_make_program(vm->control));
                stack_push(vm->stack, new_kin);
                vm->control = NULL;
                vm_populate(vm, kin_extract_tree(kin));
                break;
            }
            default: {
                fatal("vm_step: invalid tree type %d\n",
                    tree_get_type(kin_get_tree(kin)));
            }
        }
    }
    return EvalRunning;
}

struct Program* vm_run(struct VM* vm) {
    size_t step = 0; // DEBUG
    enum EvalState state = EvalRunning;
    while (state == EvalRunning) {
        state = vm_step(vm);
        printf("Step %llu\n", step++);
    }
    return vm->control;
}

enum EvalState vm_run_steps(struct VM* vm, size_t step_count) {
    size_t step = 0;
    enum EvalState state = EvalRunning;
    while (state == EvalRunning && step < step_count) {
        state = vm_step(vm);
        printf("Step %llu / %llu\n", step++, step_count);
    }
    return state;
}

struct VMData vm_serialize(struct VM* vm, uint8_t word_size) {
    assert(word_size <= sizeof(size_t));
    printf("Serializing VM\n");
    Serializer_t* serializer = serializer_init(word_size);

    program_serialize(serializer, vm->control);
    printf("Control: "); program_print(vm->control); printf("\n");
    serializer_write_word(serializer, stack_count(vm->stack));
    for (size_t i = 0; i < stack_count(vm->stack); i++) {
        kin_serialize(serializer, stack_get_elem(vm->stack, i));
        printf("Kin #%llu [%d]: ", i,
            kin_is_parent(stack_get_elem(vm->stack, i)));
        tree_print(kin_get_tree(stack_get_elem(vm->stack, i))); printf("\n");
    }

    struct VMData vm_data;
    vm_data.data = serializer_get_data(serializer);
    vm_data.data_size = serializer_get_data_size(serializer);
    serializer_free_toplevel(serializer);
    return vm_data;
}

struct VM* vm_deserialize(uint8_t* bytes) {
    Serializer_t* serializer = serializer_from_data(bytes);

    struct VM* vm = vm_make();
    vm->stack = stack_make();
    vm->control = program_deserialize(serializer);
    size_t stack_count = serializer_read_word(serializer);
    for (size_t i = 0; i < stack_count; i++) {
        stack_push(vm->stack, kin_deserialize(serializer));
    }

    return vm;
}

void vm_reset(struct VM* vm) {
    program_free(vm->control);
    vm->control = NULL;
    while (!stack_is_empty(vm->stack)) {
        kin_free(stack_pop(vm->stack));
    }
    stack_free(vm->stack);
    vm->stack = NULL;
}

void vm_free(struct VM* vm) {
    vm_reset(vm);
    free_mem("vm_free", vm);
}
