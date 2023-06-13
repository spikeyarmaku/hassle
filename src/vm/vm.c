#include "vm.h"

#include "heap.h"
#include "stack.h"
#include "closure.h"
#include "term.h"

struct VM {
    Closure_t*  control;
    Stack_t*    stack;
    Heap_t*     heap;
};

void    _vm_invoke_upd  ();
void    _vm_invoke_lam1 ();
void    _vm_invoke_lam2 ();
void    _vm_invoke_app1 ();
void    _vm_invoke_app2 ();
void    _vm_invoke_var  ();
void    _vm_invoke_int  ();
void    _vm_invoke_op1  ();
void    _vm_invoke_op2  ();

VM_t* vm_init(Expr_t* expr) {
    VM_t* vm = (VM_t*)allocate_mem("vm_init", NULL, sizeof(struct VM));
    
    vm->stack = stack_make();
    vm->heap = heap_make();
    vm->control = NULL;

    vm_set_control_to_expr(vm, expr); expr = NULL;
    
    assert(expr == NULL);
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

    // TODO Does stack need to be emptied as well?
    assert(expr == NULL);
}

enum EvalState vm_step(VM_t* vm) {
    // Update a value at a certain environment frame with the current closure
    // (Upd)  〈v, σ u, μ〉→CE〈v, σ, μ(u → v · l)〉 where c · l = μ(u)
    Closure_t* stack_top = stack_peek(vm->stack, 0);
    if (closure_get_frame(stack_top) == NULL) {
        _vm_invoke_upd();
    }

    // Introduce a new frame into the environment
    // (Lam1) 〈λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → c · l]〉 f ∈ dom(μ)
    // (Lam2) 〈Λi.t[l], σ c, μ〉 →CE〈t[f], σ, μ[(i, f) → encode(c) · l]〉
    //     (f ∈ dom(μ))
    Closure_t* control = vm->control;
    Term_t* term = closure_get_term(control);
    switch (term_get_type(term)) {
        case AbsTerm: {
            _vm_invoke_lam1();
            break;
        }
        case SyntaxTerm: {
            _vm_invoke_lam2();
            break;
        }

    // Put the second term onto the stack and make the first term the current
    // closure
    // (App1) 〈t t′[l], σ, μ〉→CE〈t[l], σ t′[l], μ〉
    // (App2) 〈!(t t′)[l], σ, μ〉→CE〈t′[l], σ t[l], μ〉
        case LazyAppTerm: {
            _vm_invoke_app1();
            break;
        }
        case StrictAppTerm: {
            _vm_invoke_app2();
            break;
        }

    // Look up the var's value and make it the current closure, while putting
    // its location onto the stack
    // (Var)  〈0[l], σ, μ〉→CE〈c, σ l, μ〉 where c · l′ = μ(l)
        case PrimvalTerm: {
            PrimVal_t* primval = term_get_primval(term);
            if (primval_get_type(primval) == SymbolValue) {
                _vm_invoke_var();
            } else {
                _vm_invoke_int();
            }
            break;
        }

    // literals and operators addition:

    // Swap the current closure and the top of the stack
    // (Int)  〈n[l], σ c, μ, k〉→CE〈c, σ n[l], μ, k〉

    // Take the necessary amount of items off the stack, and apply them with the
    // current operator
    // (Op1)  〈op[l], σ n′ n, μ, k〉→CE〈op(n′, n)[l], σ, μ, k〉
    // (Op2)  〈op[l], σ t′ t, μ, k〉→CE〈!(!(op n′) n)[l], σ, μ, k〉
        case OpTerm: {
            Closure_t* stack_top_1 = stack_peek(vm->stack, 0);
            Closure_t* stack_top_2 = stack_peek(vm->stack, 1);
            if (term_get_type(closure_get_term(stack_top_1)) == PrimvalTerm &&
                term_get_type(closure_get_term(stack_top_2)) == PrimvalTerm)
            {
                _vm_invoke_op1();
            } else {
                _vm_invoke_op2();
            }
            break;
        }

        case WorldTerm: {
            // TODO
            break;
        }
    }

    return EvalFinished;
}

Term_t* vm_run(VM_t* vm) {
    enum EvalState state = EvalRunning;
    while (state != EvalFinished) {
        state = vm_step(vm);
    }
    return closure_get_term(vm->control);
}

uint8_t* vm_save(VM_t* vm) {
    // TODO
    return NULL;
}

VM_t* vm_load(uint8_t* bytes) {
    // TODO
    return NULL;
}

void vm_free(VM_t* vm) {
    assert(vm != NULL);
    closure_free(vm->control);
    stack_free(vm->stack);
    heap_free(vm->heap);
    free_mem("vm_free", vm);
}

// ------------------------- VM invocations ------------------------------------

void _vm_invoke_upd() {
    //
}

void _vm_invoke_lam1() {
    //
}

void _vm_invoke_lam2() {
    //
}

void _vm_invoke_app1() {
    //
}

void _vm_invoke_app2() {
    //
}

void _vm_invoke_var() {
    //
}

void _vm_invoke_int() {
    //
}

void _vm_invoke_op1() {
    //
}

void _vm_invoke_op2() {
    //
}

