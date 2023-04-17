#include "expr.h"

struct Expr {
  enum ExprType type;
  union {
    struct {
        size_t child_count;
        size_t capacity;
        Expr_t* children;
    };
    char* symbol;
  };
};

Expr_t  expr_make_empty        ();
void    expr_set_as_atom       (Expr_t, char*);
void    expr_set_as_list       (Expr_t);

Expr_t expr_make_empty() {
    return allocate_mem("expr_make_empty", NULL, sizeof(struct Expr));
}

void expr_set_as_atom(Expr_t expr, char* symbol) {
    expr->type = ExprAtom;
    expr->symbol = str_cpy(symbol);
}

void expr_set_as_list(Expr_t expr) {
    expr->type = ExprList;
    expr->children = NULL;
    expr->child_count = 0;
    expr->capacity = 0;
}

void expr_add_to_list(Expr_t list_expr, Expr_t child) {
    debug_start("expr_add_to_list\n");
    assert(list_expr->type == ExprList);
    
    if (list_expr->capacity == list_expr->child_count) {
        // Allocate memory
        list_expr->children = (Expr_t*)allocate_mem("expr_add_to_list",
            list_expr->children,
            sizeof(Expr_t) * (list_expr->capacity + EXPR_BUFFER_SIZE));
        assert(list_expr->children != NULL);
        list_expr->capacity += EXPR_BUFFER_SIZE;
    }

    list_expr->children[list_expr->child_count] = child;
    list_expr->child_count++;
    debug_end("/expr_add_to_list\n");
}

Expr_t expr_make_atom(char* symbol) {
    Expr_t result = expr_make_empty();
    assert(result != NULL);
    expr_set_as_atom(result, symbol);
    return result;
}

Expr_t expr_make_empty_list() {
    Expr_t result = expr_make_empty();
    assert(result != NULL);
    expr_set_as_list(result);
    return result;
}

Expr_t expr_get_child(Expr_t expr, size_t index) {
    if (expr == NULL) return NULL;
    if (index >= expr->child_count) return NULL;
    return expr->children[index];
}

size_t expr_get_child_count(Expr_t expr) {
    return expr->child_count;
}

BOOL expr_is_list(Expr_t expr) {
    if (expr == NULL) return FALSE;
    return expr->type == ExprList ? TRUE : FALSE;
}

BOOL expr_is_empty_list(Expr_t expr) {
    if (expr_is_list(expr)) {
        return expr->child_count == 0;
    } else {
        return FALSE;
    }
}

Expr_t* expr_get_list(Expr_t expr) {
    if (expr == NULL) return NULL;
    return expr->children;
}

char* expr_get_symbol(Expr_t expr) {
    if (expr == NULL) return NULL;
    return expr->symbol;
}

// Check if two expressions are equal. Return TRUE if equal, FALSE if not.
BOOL expr_is_equal(Expr_t e1, Expr_t e2) {
    assert(e1 != NULL);
    assert(e2 != NULL);
    
    // If they have different types, return false
    if (e1->type != e2->type) return FALSE;

    if (e1->type == ExprAtom) {
        // If they are both atoms, compare the symbols
        return strcmp(e1->symbol, e2->symbol) == 0;
    } else {
        // If they are both lists, march through the elements
        size_t i = 0;
        
        Expr_t c1, c2;
        do {
            c1 = expr_get_child(e1, i);
            c2 = expr_get_child(e2, i);
            if (!expr_is_equal(c1, c2)) {
                return FALSE;
            }
            i++;
        } while (c1 != NULL && c2 != NULL);
        return TRUE;
    }
}

// Return the number of the longest series of sub-expressions, starting from the
// start, that both expressions share. E.g. (a b (c d) e) and (a b (c f) e)
// would yield 2
size_t expr_match_size(Expr_t expr1, Expr_t expr2) {
    if (expr_is_list(expr1) != expr_is_list(expr2)) {
        return 0;
    }

    if (!expr_is_list(expr1)) {
        return expr_is_equal(expr1, expr2) ? 1 : 0;
    }

    size_t counter = 0;
    Expr_t c1, c2;
    do {
        c1 = expr_get_child(expr1, counter);
        c2 = expr_get_child(expr2, counter);
        counter++;
    } while (expr_is_equal(c1, c2) && c1 != NULL && c2 != NULL);
    counter--;
    return counter;
}

char* expr_to_string(Expr_t expr) {
    // debug_start("expr_to_string\n");
    assert(expr != NULL);
    
    if (expr->type == ExprAtom) {
        // debug("expr_to_string - Atom: %s\n", expr->symbol);
        char* result = str_cpy(expr->symbol);
        // debug_end("/expr_to_string - Atom\n");
        return result;
    } else {
        // debug("expr_to_string - List\n");
        
        // Store the children's strings and count the sum of the length of their
        // string representations
        char** child_strings =
            (char**)allocate_mem("expr_to_string/list/children", NULL,
            sizeof(char*) * expr->child_count);
        assert(child_strings != NULL);
        size_t length = 0;
        for (size_t i = 0; i < expr->child_count; i++) {
            child_strings[i] = expr_to_string(expr->children[i]);
            length += strlen(child_strings[i]);
        }

        // Leave enough space for the parens (2), the spaces between the
        // children (child_count - 1) and for the terminating null (1)
        length += 2 + (expr->child_count - 1) + 1;

        // Concatenate the children's strings into one string
        char* result = (char*)allocate_mem("expr_to_string/list/result", NULL,
            sizeof(char) * length);
        result[0] = 0;
        strcat(result, "(");
        for (size_t i = 0; i < expr->child_count; i++) {
            strcat(result, child_strings[i]);
            if (i < expr->child_count - 1) {
                strcat(result, " ");
            }
        }
        strcat(result, ")");

        // Free up the children's strings
        for (size_t i = 0; i < expr->child_count; i++) {
            free_mem("expr_to_string/list/children", child_strings[i]);
        }
        free_mem("expr_to_string/list/children_ptr", child_strings);

        // Return the result
        // debug_end("/expr_to_string - List\n");
        return result;
    }
}

void expr_print(Expr_t expr) {
    debug_off();
    char* str = expr_to_string(expr);
    debug_on();
    printf("%s", str);
    free_mem("expr_print", str);
}

void expr_free(Expr_t* expr_ptr) {
    debug_start("expr_free\n");
    if (expr_ptr == NULL) {
        debug_end("/expr_free\n");
        return;
    }
    Expr_t expr = *expr_ptr;
    if (expr == NULL) {
        debug_end("/expr_free\n");
        return;
    }
    // expr_print(expr);
    if (expr->type == ExprAtom) {
        free_mem("expr_free/symbol", expr->symbol);
        *expr_ptr = NULL;
    } else {
        for (size_t i = 0; i < expr->child_count; i++) {
            expr_free(&(expr->children[i]));
        }
        free_mem("expr_free/children", expr->children);
    }
    free_mem("expr_free/expr", expr);
    *expr_ptr = NULL;
    debug_end("/expr_free\n");
}

Expr_t expr_copy(Expr_t expr) {
    debug_start("expr_copy - %llu\n", (size_t)expr); // expr_print(expr); debug("\n");
    // Expr can be null when copying, e.g. if a closure hasn't been fully
    // completed yet, like with lambda_helper2
    if (expr == NULL) {
        debug_end("/expr_copy\n");
        return NULL;
    }

    Expr_t result;
    if (expr->type == ExprAtom) {
        result = expr_make_atom(expr->symbol);
        assert(result != NULL);
        debug_end("/expr_copy\n");
        return result;
    } else {
        result = expr_make_empty_list();
        assert(result != NULL);
        for (size_t i = 0; i < expr->child_count; i++) {
            expr_add_to_list(result, expr_copy(expr_get_child(expr, i)));
        }
        debug_end("/expr_copy\n");
        return result;
    }
}
