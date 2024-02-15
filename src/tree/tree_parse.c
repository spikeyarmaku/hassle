#include "tree_parse.h"

struct TreeStack {
    size_t capacity;
    size_t count;
    struct Tree** elems;
};

void                _tree_stack_grow    (struct TreeStack*);
struct TreeStack*   _tree_stack_create  ();
void                _tree_stack_push    (struct TreeStack*, struct Tree*);
struct Tree*        _tree_stack_pop     (struct TreeStack*);

// ----------------------------------------------------------------------------

void _tree_stack_grow(struct TreeStack* stack) {
    size_t new_size = stack->capacity < BUFFER_SIZE ?
        BUFFER_SIZE : stack->capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    stack->elems = allocate_mem("_tree_stack_grow", stack->elems,
        sizeof(struct Tree*) * new_size);
    stack->capacity = new_size;
}

struct TreeStack* _tree_stack_create() {
    struct TreeStack* stack = allocate_mem("_tree_stack_create", NULL,
        sizeof(struct TreeStack));
    stack->capacity = 0;
    stack->count = 0;
    stack->elems = NULL;
    return stack;
}

void _tree_stack_push(struct TreeStack* stack, struct Tree* tree) {
    if (stack->capacity == stack->count) {
        _tree_stack_grow(stack);
    }

    stack->elems[stack->count] = tree;
    stack->count++;
}

struct Tree* _tree_stack_pop(struct TreeStack* stack) {
    stack->count--;
    return stack->elems[stack->count];
}

// ----------------------------------------------------------------------------

BOOL _is_opening_paren(char p) {
    return (p == '[' || p == '{' || p == '(' || p == '<') ? TRUE : FALSE;
}

BOOL _is_closing_paren(char p) {
    return (p == ']' || p == '}' || p == ')' || p == '>') ? TRUE : FALSE;
}

BOOL _is_paren(char p) {
    return (_is_opening_paren(p) == TRUE || _is_closing_paren(p) == TRUE) ?
        TRUE : FALSE;
}

struct Tree* tree_from_string(char* str) {
    // *(**)(**)
    // (*(**))**
    struct TreeStack* stack = _tree_stack_create();
    struct Tree* tree = NULL;
    size_t i = 0;
    char c = *str;
    while (c != 0) {
        if (_is_opening_paren(c) == TRUE) {
            _tree_stack_push(stack, tree);
            tree = NULL;
        } else {
            if (_is_closing_paren(c) == TRUE) {
                struct Tree* temp = _tree_stack_pop(stack);
                tree = tree_apply(temp, tree);
            } else {
                //
                if (tree == NULL) {
                    tree = tree_make_program(program_make_leaf());
                } else {
                    tree = tree_apply(tree,
                        tree_make_program(program_make_leaf()));
                }
            }
        }
        
        i++;
        c = *(str + i);
    }
    return tree;
}
