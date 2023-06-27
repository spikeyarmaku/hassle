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

VM_t*   _vm_make        (Stack_t*, Heap_t*, Closure_t*);
void    _vm_invoke_upd  (VM_t*);
void    _vm_invoke_lam  (VM_t*, BOOL);
void    _vm_invoke_app  (VM_t*, BOOL);
void    _vm_invoke_var  (VM_t*);
void    _vm_invoke_int  (VM_t*);
void    _vm_invoke_op   (VM_t*);

VM_t* _vm_make(Stack_t* stack, Heap_t* heap, Closure_t* control) {
    VM_t* vm = (VM_t*)allocate_mem("vm_init", NULL, sizeof(struct VM));

    vm->stack = stack;
    vm->heap = heap;
    vm->control = control;

    return vm;
}

VM_t* vm_init() {
    VM_t* vm = _vm_make(stack_make(), heap_make_default(), NULL);

    // vm_set_control_to_expr(vm, expr); expr = NULL;

    return vm;
}

void vm_set_control_to_expr(VM_t* vm, Expr_t* expr) {
    Term_t* term = term_from_expr(expr);
    expr_free(expr); expr = NULL;
    
    if (vm->control != NULL) {
        closure_free(vm->control);
    }
    vm->control = closure_make(term, heap_get_current_frame(vm->heap));
    term = NULL;

    // NOTE Does stack need to be emptied as well?
    assert(expr == NULL);
}

enum EvalState vm_step(VM_t* vm) {
    printf("Checking UPD...\n");
    // Update a value at a certain environment frame with the current closure
    // (Upd)  〈v, σ u, μ〉→CE〈v, σ, μ(u → v · l)〉 where c · l = μ(u)
    Closure_t* stack_top = stack_peek(vm->stack, 0);
    if (stack_top != NULL) {
        if (closure_is_update(stack_top) == TRUE) {
            _vm_invoke_upd(vm);
        }
    }

    // Introduce a new frame into the environment
    // (Lam) 〈λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → c · l]〉 f ∈ dom(μ)
    //     (f ∈ dom(μ))
    Closure_t* control = vm->control;
    Term_t* term = closure_get_term(control);
    switch (term_get_type(term)) {
        case AbsTerm: {
            printf("LAM...\n");
            _vm_invoke_lam(vm, TRUE);
            break;
        }
        // case SyntaxTerm: {
        //     _vm_invoke_lam(vm, FALSE);
        //     break;
        // }

    // Put the second term onto the stack and make the first term the current
    // closure
    // (App1) 〈t t′[l], σ, μ〉→CE〈t[l], σ t′[l], μ〉
    // (App2) 〈!(t t′)[l], σ, μ〉→CE〈t′[l], σ t[l], μ〉
        case LazyAppTerm: {
            printf("APP1...\n");
            _vm_invoke_app(vm, TRUE);
            break;
        }
        case StrictAppTerm: {
            printf("APP2...\n");
            _vm_invoke_app(vm, FALSE);
            break;
        }

    // Look up the var's value and make it the current closure, while putting
    // its location onto the stack
    // (Var)  〈0[l], σ, μ〉→CE〈c, σ l, μ〉 where c · l′ = μ(l)
    // literals and operators addition:
    // Swap the current closure and the top of the stack
    // (Int)  〈n[l], σ c, μ, k〉→CE〈c, σ n[l], μ, k〉
        case PrimvalTerm: {
            PrimVal_t* primval = term_get_primval(term);
            if (primval_get_type(primval) == SymbolValue) {
                printf("VAR...\n");
                _vm_invoke_var(vm);
            } else {
                printf("INT...\n");
                _vm_invoke_int(vm);
            }
            break;
        }

    // Take the necessary amount of items off the stack, and apply them with the
    // current operator
    // (Op1)  〈op[l], σ n′ n, μ, k〉→CE〈op(n′, n)[l], σ, μ, k〉
    // (Op2)  〈op[l], σ t′ t, μ, k〉→CE〈!(!(op n′) n)[l], σ, μ, k〉
        case OpTerm: {
            printf("OP...\n");
            _vm_invoke_op(vm);
            break;
        }

        case WorldTerm: {
            // TODO
            break;
        }
    }

    stack_top = stack_peek(vm->stack, 0);
    printf("CHECK: control: %d, stack_top: %llu\n",
        term_get_type(closure_get_term(vm->control)), (size_t)stack_top);
    if (term_get_type(closure_get_term(vm->control)) == PrimvalTerm &&
        stack_top == NULL)
    {
        printf("FINISHED\n");
        return EvalFinished;
    }

    return EvalRunning;
}

Term_t* vm_run(VM_t* vm) {
    enum EvalState state = EvalRunning;
    while (state != EvalFinished) {
        state = vm_step(vm);
    }
    return closure_get_term(vm->control);
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
    printf("Serializing the heap\n");
    printf("vm: %llu\n", (size_t)vm);
    heap_serialize(serializer, vm->heap);
    
    // Save the control
    printf("Serializing the control\n");
    closure_serialize(serializer, vm->heap, vm->control);
    
    // Save the stack
    printf("Serializing the stack\n");
    stack_serialize(serializer, vm->heap, vm->stack);
    
    struct VMData vm_data;
    vm_data.data = serializer_get_data(serializer);
    vm_data.data_size = serializer_get_data_size(serializer);

    // if word_size is the same as the word size for this architecture, this
    // condition will always be false
    if (vm_data.data_size > (1 << word_size)) {
        serializer_free(serializer); serializer = NULL;
        vm_data.data = NULL;
        vm_data.data_size = 0;
    }

    return vm_data;
}

// Deserialize the VM
// Note that when Frames are deserialized, the pointer for their parent needs to
// be read from the Heap, as the serialized version contains only the index for
// their parent.
VM_t* vm_deserialize(uint8_t* bytes) {
    Serializer_t* serializer = serializer_from_data(bytes);
    
    printf("deserealizing the heap\n");
    Heap_t* heap = heap_deserialize(serializer);
    printf("deserealizing the control\n");
    Closure_t* control = closure_deserialize(serializer, heap);
    printf("deserealizing the stack\n");
    Stack_t* stack = stack_deserialize(serializer, heap);
    return _vm_make(stack, heap, control);
}

void vm_free(VM_t* vm) {
    assert(vm != NULL);
    closure_free(vm->control);
    stack_free(vm->stack);
    heap_free(vm->heap);
    free_mem("vm_free", vm);
}

void vm_reset(VM_t* vm) {
    closure_free(vm->control);  vm->control = NULL;
    stack_free(vm->stack);      vm->stack = stack_make();
    heap_free(vm->heap);        vm->heap = heap_make_default();
}

// ------------------------- VM invocations ------------------------------------

// (Upd)  〈v, σ u, μ〉→CE〈v, σ, μ(u → v · l)〉 where c · l = μ(u)
// Update the bound term in the frame with its evaluated value
void _vm_invoke_upd(VM_t* vm) {
    Closure_t* stack_top = stack_pop(vm->stack);
    Frame_t* frame = closure_get_frame(stack_top);
    closure_free(stack_top); stack_top = NULL;
    Closure_t* control = vm->control;
    frame_update(frame, closure_copy(control));
}

// (Lam) 〈λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → c · l]〉 f ∈ dom(μ)
//     (f ∈ dom(μ))
// Bind the lambda's variable to the (encoded) value on the top of the stack
void _vm_invoke_lam(VM_t* vm, BOOL encode) {
    // Pop the closure from the top of the stack
    Closure_t* stack_top = stack_pop(vm->stack);
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);
    Frame_t* control_frame = closure_get_frame(control);
    
    assert(term_get_type(control_term) == AbsTerm);

    // Deconstruct the control's content and put back the body of the lambda
    char* var_name = term_get_abs_var(control_term);
    Term_t* abs_term = term_get_abs_body(control_term);
    vm->control = closure_make(abs_term, control_frame);
    closure_free_toplevel(control); control = NULL;
    term_free_toplevel(control_term); control_term = NULL;

    if (encode == TRUE) {
        Frame_t* stack_top_frame = closure_get_frame(stack_top);
        Term_t* stack_top_term = closure_get_term(stack_top);
        stack_top = closure_make(term_encode_as_list(stack_top_term),
            stack_top_frame);
    }

    // Bind the variable to the item that was popped off the stack
    heap_add(vm->heap, frame_make(var_name, stack_top,
        heap_get_current_frame(vm->heap)));
}

// (App1) 〈t t′[l], σ, μ〉→CE〈t[l], σ t′[l], μ〉
// (App2) 〈!(t t′)[l], σ, μ〉→CE〈t′[l], σ t[l], μ〉
// Put the right (is_lazy) or the left side (!is_lazy) of the app to the stack
void _vm_invoke_app(VM_t* vm, BOOL is_lazy) {
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);
    
    assert(term_get_type(control_term) == LazyAppTerm ||
        term_get_type(control_term) == StrictAppTerm);

    Term_t* term1 = term_get_app_term1(control_term);
    Term_t* term2 = term_get_app_term2(control_term);
    if (is_lazy == FALSE) {
        Term_t* temp = term1;
        term1 = term2;
        term2 = temp;
    }

    vm->control = closure_make(term1, closure_get_frame(control));
    closure_free_toplevel(control); control = NULL;
    term_free_toplevel(control_term); control_term = NULL;

    stack_add_closure(vm->stack,
        closure_make(term2, heap_get_current_frame(vm->heap)));
}

// (Var)  〈0[l], σ, μ〉→CE〈c, σ l, μ〉 where c · l′ = μ(l)
// Look up the value of the variable in its env
void _vm_invoke_var(VM_t* vm) {
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);

    assert(term_get_type(control_term) == PrimvalTerm);

    PrimVal_t* primval = term_get_primval(control_term);

    assert(primval_get_type(primval) == SymbolValue);

    Closure_t* value = frame_lookup(closure_get_frame(control),
        primval_get_symbol(primval));
    // TODO check if frame_lookup doesn't return a useful value (if applicable)
    closure_free(control); control = NULL;
    vm->control = value;
}

// (Int)  〈n[l], σ c, μ, k〉→CE〈c, σ n[l], μ, k〉
// Swap the contents of the control and the top of the stack
void _vm_invoke_int(VM_t* vm) {
    Closure_t* control = vm->control;
    Closure_t* stack_top = stack_pop(vm->stack);
    stack_add_closure(vm->stack, control);
    vm->control = stack_top;
}

// (Op1)  〈op[l], σ n′ n, μ, k〉→CE〈op(n′, n)[l], σ, μ, k〉
// (Op2)  〈op[l], σ t′ t, μ, k〉→CE〈!(!(op n′) n)[l], σ, μ, k〉
// Apply the operator to the topmost elements of the stack
void _vm_invoke_op(VM_t* vm) {
    Closure_t* control = vm->control;
    Term_t* control_term = closure_get_term(control);

    assert(term_get_type(control_term));
    
    enum PrimOp op = term_get_op(control_term);
    uint8_t op_arity = primop_get_arity(op);
    closure_free_toplevel(control); control = NULL; vm->control = NULL;

    // Pop off n items from stack and check if they are all values
    Closure_t** args = (Closure_t**)allocate_mem("_vm_invoke_op1", NULL,
        (sizeof(Closure_t*) * op_arity));
    BOOL all_values = TRUE;
    for (uint8_t i = 0; i < op_arity; i++) {
        args[i] = stack_pop(vm->stack);
        all_values = all_values &
            (term_get_type(closure_get_term(args[i])) == PrimvalTerm);
    }

    if (all_values) {
        // Apply the operator to the args
        vm->control = primop_apply(op, args);
    } else {
        // Construct a strict app of the operator and the args
        Term_t* result = control_term;
        for (uint8_t i = 0; i < op_arity; i++) {
            result = term_make_strict_app(result, closure_get_term(args[i]));
            closure_free_toplevel(args[i]);
        }
        vm->control = closure_make(result, heap_get_current_frame(vm->heap));
    }

    // Free up the argument array, but not its constituents
    free_mem("_vm_invoke_op1", args);
}


