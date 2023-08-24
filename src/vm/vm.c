#include "vm.h"

#include "heap.h"
#include "stack.h"
#include "closure.h"
#include "term.h"
#include "primop.h"
#include "serialize\serialize.h"

struct VM {
    Closure_t*  control;
    Stack_t*    stack;
    Heap_t*     heap;
};

VM_t*           _vm_make        (Stack_t*, Heap_t*, Closure_t*);
void            _vm_invoke_upd  (VM_t*);
void            _vm_invoke_lam  (VM_t*);
void            _vm_invoke_app  (VM_t*);
void            _vm_invoke_var  (VM_t*);
void            _vm_invoke_val  (VM_t*);
void            _vm_invoke_op   (VM_t*);
enum EvalState  _vm_check_state (VM_t* vm);

VM_t* _vm_make(Stack_t* stack, Heap_t* heap, Closure_t* control) {
    VM_t* vm = (VM_t*)allocate_mem("vm_init", NULL, sizeof(struct VM));

    vm->stack = stack;
    vm->heap = heap;
    vm->control = control;

    return vm;
}

VM_t* vm_init() {
    VM_t* vm = _vm_make(stack_make(), heap_make_default(), NULL);
    return vm;
}

Term_t* _test_expr() {
    // (\a.(\b.(b a) \c.(c a)) (\i.i \j.j))
    return
        term_make_app(
            term_make_abs("a",
                    term_make_app(
                        term_make_abs("b",
                            term_make_app(
                                term_make_primval_reference("b"),
                                term_make_primval_reference("a"))),
                        term_make_abs("c",
                            term_make_app(
                                term_make_primval_reference("c"),
                                term_make_primval_reference("a"))))),
            term_make_app(
                term_make_abs("i", term_make_primval_reference("i")),
                term_make_abs("j", term_make_primval_reference("j"))));
    // TODO test the Y combinator and how it destroys sharing
}

void vm_set_control_to_expr(VM_t* vm, Expr_t* expr) {
    Term_t* term =
        // term_from_expr(expr);
        term_make_app(term_make_primval_reference("eval"),
            term_from_expr_encoded(expr));
        // _test_expr();
    // term_print(term); printf("\n");
    // expr_free(expr); expr = NULL;
    
    // if (vm->control != NULL) {
    //     closure_free(vm->control);
    // }
    vm->control = closure_make(term, heap_get_current_frame(vm->heap));
    term = NULL;

    // NOTE Does stack need to be emptied as well?
    // assert(expr == NULL);
}

enum EvalState _vm_check_state(VM_t* vm) {
    if (vm->control == NULL) {
        return EvalFinished;
    }
    if (stack_peek(vm->stack, 0) == NULL &&
        term_is_self_evaluating(closure_get_term(vm->control)))
    {
        return EvalFinished;
    }
    // If the stack's topmost two elements are both the same updates, we're done
    if (stack_get_elem_count(vm->stack) >= 2) {
        Closure_t* stack_0 = stack_peek(vm->stack, 0);
        Closure_t* stack_1 = stack_peek(vm->stack, 1);
        if (closure_is_update(stack_0) && closure_is_update(stack_1) &&
            closure_get_frame(stack_0) == closure_get_frame(stack_1))
        {
            return EvalFinished;
        }
    }

    return EvalRunning;
}

enum EvalState vm_step(VM_t* vm) {
    // Update a self-evaluating value at a certain environment frame with the
    // current closure
    // (Upd)  〈v, σ u, μ〉→CE〈v, σ, μ(u → v · l)〉 where c · l = μ(u)
    Closure_t* stack_top = stack_peek(vm->stack, 0);
    if (stack_top != NULL) {
        if ((term_is_self_evaluating(closure_get_term(vm->control)) &&
            closure_is_update(stack_top)) == TRUE)
        {
            _vm_invoke_upd(vm);
            return _vm_check_state(vm);
        }
    }

    // Introduce a new frame into the environment
    // (Lam) 〈λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → c · l]〉 f ∈ dom(μ)
    //     (f ∈ dom(μ))
    Closure_t* control = vm->control;

    if (closure_get_frame(control) == NULL) {
        // If the closure has no valid frame, treat it as a value
        _vm_invoke_val(vm);
    } else {
        Term_t* term = closure_get_term(control);
        switch (term_get_type(term)) {
            case AbsTerm: {
                if (stack_top != NULL) {
                    _vm_invoke_lam(vm);
                } else {
                    return EvalFinished;
                }
                break;
            }

    // Put the second term onto the stack and make the first term the current
    // closure
    // (App) 〈t t′[l], σ, μ〉→CE〈t[l], σ t′[l], μ〉
            case AppTerm: {
                _vm_invoke_app(vm);
                break;
            }

    // Look up the var's value and make it the current closure, while putting
    // its location onto the stack
    // (Var)  〈0[l], σ, μ〉→CE〈c, σ l, μ〉 where c · l′ = μ(l)
    // literals and operators addition:
    // Swap the current closure and the top of the stack
    // (Val)  〈n[l], σ c, μ, k〉→CE〈c, σ n[l], μ, k〉
            case PrimValTerm: {
                // PrimVal_t* primval = term_get_primval(term);
                if (term_is_self_evaluating(term) == FALSE) {
                // if (primval_get_type(primval) == ReferenceValue) {
                    _vm_invoke_var(vm);
                } else {
                    if (stack_top != NULL) {
                        _vm_invoke_val(vm);
                    } else {
                        return EvalFinished;
                    }
                }
                break;
            }

    // Take the necessary amount of items off the stack, and apply them with the
    // current operator
    // (Op1)  〈op[l], σ n′ n, μ, k〉→CE〈op(n′, n)[l], σ, μ, k〉
            case OpTerm: {
                if (stack_top != NULL) {
                    _vm_invoke_op(vm);
                } else {
                    return EvalFinished;
                }
                break;
            }

            case DummyTerm: {
                _vm_invoke_val(vm);
            }
        }
    }

    return _vm_check_state(vm);
}

Term_t* vm_run(VM_t* vm) {
    enum EvalState state = EvalRunning;
    while (state != EvalFinished) {
        state = vm_step(vm);
    }
    return term_copy(closure_get_term(vm->control));
}

// Serialize the VM
// Note that when Frames are serialized, the index of their parent needs to be
// written instead of the actual pointer. However, we can't use the index in
// the actual program, as looking up a name would then require a Heap instance
// as well, to find the parent Frame.
struct VMData vm_serialize(VM_t* vm, uint8_t word_size) {
    assert(word_size <= sizeof(size_t));
    Serializer_t* serializer = serializer_init(word_size);

    // Save the heap
    heap_serialize(serializer, vm->heap);
    // printf("Serializer after heap:\n");
    // serializer_print(serializer); printf("\n");
    
    // Save the control
    closure_serialize(serializer, vm->heap, vm->control);
    // printf("Serializer after closure:\n");
    // serializer_print(serializer); printf("\n");
    
    // Save the stack
    stack_serialize(serializer, vm->heap, vm->stack);
    // printf("Serializer after stack:\n");
    // serializer_print(serializer); printf("\n\n");
    
    struct VMData vm_data;
    vm_data.data = serializer_get_data(serializer);
    vm_data.data_size = serializer_get_data_size(serializer);
    // serializer_free_toplevel(serializer);

    // if word_size is the same as the word size for this architecture, this
    // condition will always be false
    // if (vm_data.data_size > (1 << word_size)) {
    //     serializer_free(serializer); serializer = NULL;
    //     vm_data.data = NULL;
    //     vm_data.data_size = 0;
    // }

    return vm_data;
}

// Deserialize the VM
// Note that when Frames are deserialized, the pointer for their parent needs to
// be read from the Heap, as the serialized version contains only the index for
// their parent.
VM_t* vm_deserialize(uint8_t* bytes) {
    Serializer_t* serializer = serializer_from_data(bytes);
    
    Heap_t* heap = heap_deserialize(serializer);
    Closure_t* control = closure_deserialize(serializer, heap);
    Stack_t* stack = stack_deserialize(serializer, heap);
    return _vm_make(stack, heap, control);
}

// void vm_free(VM_t* vm) {
//     assert(vm != NULL);
//     closure_free(vm->control);
//     stack_free(vm->stack);
//     heap_free(vm->heap);
//     free_mem("vm_free", vm);
// }

void vm_reset(VM_t* vm) {
    // closure_free(vm->control);  vm->control = NULL;
    // stack_free(vm->stack);
    vm->stack = stack_make();
    // heap_free(vm->heap);
    vm->heap = heap_make_default();
}

// ------------------------- VM invocations ------------------------------------

// (Upd)  〈v, σ u, μ〉→CE〈v, σ, μ(u → v · l)〉 where c · l = μ(u)
// Update the bound term in the frame with its evaluated value
void _vm_invoke_upd(VM_t* vm) {
    printf("[UPD]\n");
    Closure_t* stack_top = stack_pop(vm->stack);
    Frame_t* frame = closure_get_frame(stack_top);
    // closure_free(stack_top); stack_top = NULL;
    Closure_t* control = vm->control;
    frame_update(frame, closure_copy(control));
}

// (Lam) 〈λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → c · l]〉 f ∈ dom(μ)
//     (f ∈ dom(μ))
// Bind the vau's variable to the (encoded) value on the top of the stack
void _vm_invoke_lam(VM_t* vm) {
    printf("[LAM]\n");
    // Pop the closure from the top of the stack
    Closure_t* stack_top = stack_pop(vm->stack);
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);

    assert(term_get_type(control_term) == AbsTerm);

    // Deconstruct the control's content and put back the body of the vau
    char* var_name = term_get_abs_var(control_term);
    
    // Bind the variable to the item that was popped off the stack
    heap_add(vm->heap, frame_make(var_name, stack_top,
        closure_get_frame(control)));

    Term_t* abs_term = term_get_abs_body(control_term);
    vm->control = closure_make(abs_term, heap_get_current_frame(vm->heap));
    // closure_free_toplevel(control); control = NULL;
    // term_free_toplevel(control_term); control_term = NULL;

}

// (App) 〈t t′[l], σ, μ〉→CE〈t[l], σ t′[l], μ〉
// Put the right side of the app to the stack
void _vm_invoke_app(VM_t* vm) {
    printf("[APP]\n");
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);
    
    assert(term_get_type(control_term) == AppTerm);

    Term_t* term1 = term_get_app_term1(control_term);
    Term_t* term2 = term_get_app_term2(control_term);

    vm->control = closure_make(term1, closure_get_frame(control));
    // closure_free_toplevel(control); control = NULL;
    // term_free_toplevel(control_term); control_term = NULL;

    stack_add_closure(vm->stack,
        closure_make(term2, closure_get_frame(control)));
}

// (Var)  〈0[l], σ, μ〉→CE〈c, σ l, μ〉 where c · l′ = μ(l)
// Look up the var's value and make it the current closure, while putting its
// location onto the stack
void _vm_invoke_var(VM_t* vm) {
    assert(vm != NULL);
    printf("[VAR]\n");
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);

    assert(term_get_type(control_term) == PrimValTerm);

    PrimVal_t* primval = term_get_primval(control_term);

    assert(primval_get_type(primval) == ReferenceValue);

    Frame_t* parent;
    Closure_t* value = frame_lookup(closure_get_frame(control),
        primval_get_reference(primval), &parent);

    if (value == NULL) {
        vm->control =
            closure_make(
                term_make_primval_symbol(
                    primval_get_reference(term_get_primval(control_term))),
                closure_get_frame(control));
    } else {
        stack_add_update(vm->stack, parent);
        // closure_free(control); control = NULL;
        vm->control = value;
    }
}

// (Val)  〈n[l], σ c, μ, k〉→CE〈c, σ n[l], μ, k〉
// Swap the contents of the control and the top of the stack
void _vm_invoke_val(VM_t* vm) {
    printf("[VAL]\n");
    Closure_t* control = vm->control;
    Closure_t* stack_top = stack_pop(vm->stack);
    if (stack_top == NULL) {
        return;
    }
    stack_add_closure(vm->stack, control);
    vm->control = stack_top;
}

// (Op)   〈op[l], σ n′ n, μ, k〉→CE〈op(n′, n)[l], σ, μ, k〉
// Apply the operator to the topmost elements of the stack
void _vm_invoke_op(VM_t* vm) {
    printf("[OP]\n");
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);

    assert(term_get_type(control_term));
    
    enum PrimOp op = term_get_op(control_term);
    uint8_t op_arity = primop_get_arity(op);
    // closure_free_toplevel(control); control = NULL; vm->control = NULL;

    // Pop off n items from stack and check if they are all values
    Closure_t** args = (Closure_t**)allocate_mem("_vm_invoke_op1", NULL,
        (sizeof(Closure_t*) * op_arity));
    // BOOL all_values = TRUE;
    for (uint8_t i = 0; i < op_arity; i++) {
        // TODO use peek instead of pop
        args[i] = stack_pop(vm->stack);
        // all_values = all_values &
        //     (term_get_type(closure_get_term(args[i])) == PrimValTerm);
    }

    vm->control = primop_apply(op, args);

    // Free up the argument array, but not its constituents
    free_mem("_vm_invoke_op1", args);
}
