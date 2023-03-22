#include "expr.h"

Expr_t _expr_make_empty(ErrorCode_t* error_code) {
    debug(1, "_expr_make_empty\n");
    Expr_t expr = (Expr_t)allocate_mem("_expr_make_empty", NULL,
        sizeof(struct Expr));

    *error_code = (expr == NULL) ? Error : Success;
    if (*error_code == Success) {
        expr->_next = NULL;
        // expr->_prev = NULL;
    }
    debug(1, "/_expr_make_empty\n");
    return expr;
}

void _expr_set_as_atom(Expr_t expr, char* symbol) {
    expr->_type = ExprAtom;
    char* my_symbol = (char*)allocate_mem("_expr_set_as_atom", NULL,
        sizeof(char) * (strlen(symbol) + 1));
    strcpy(my_symbol, symbol);
    expr->_symbol = my_symbol;
}

void _expr_set_as_list(Expr_t expr) {
    expr->_type = ExprList;
    expr->_list = NULL;
}

void expr_add_to_list(ErrorCode_t* error_code, Expr_t list_expr, Expr_t child) {
    debug(1, "expr_add_to_list\n");
    if (list_expr->_type != ExprList) {
        *error_code = Error;
        debug(1, "/expr_add_to_list\n");
        return;
    }

    if (list_expr->_list == NULL) {
        list_expr->_list = child;
    } else {
        Expr_t last_elem = list_expr->_list;
        while (last_elem->_next != NULL) {
            last_elem = last_elem->_next;
        }
        last_elem->_next = child;
        // child->_prev = last_elem
    }
    debug(1, "/expr_add_to_list\n");
}

Expr_t expr_make_atom(ErrorCode_t* error_code, char* symbol) {
    debug(1, "expr_make_atom\n");
    Expr_t expr = _expr_make_empty(error_code);
    if (*error_code == Success) {
        _expr_set_as_atom(expr, symbol);
    }
    debug(1, "/expr_make_atom\n");
    return expr;
}

Expr_t expr_make_empty_list(ErrorCode_t* error_code) {
    debug(1, "expr_make_empty_list\n");
    Expr_t expr = _expr_make_empty(error_code);
    if (*error_code == Success) {
        _expr_set_as_list(expr);
    }
    debug(1, "/expr_make_empty_list\n");
    return expr;
}

Expr_t expr_get_next(Expr_t expr) {
    if (expr == NULL) return NULL;
    return expr->_next;
}

// Expr_t expr_get_prev(Expr_t expr) {
//     if (expr == NULL) return NULL;
//     return expr->_prev;
// }

BOOL expr_is_list(Expr_t expr) {
    if (expr == NULL) return FALSE;
    return expr->_type == ExprList;
}

Expr_t expr_get_list(Expr_t expr) {
    if (expr == NULL) return NULL;
    return expr->_list;
}

char* expr_get_symbol(Expr_t expr) {
    if (expr == NULL) return NULL;
    return expr->_symbol;
}

// Check if two expressions are equal. Return TRUE if equal, FALSE if not.
BOOL expr_is_equal(Expr_t e1, Expr_t e2) {
    // If they have different types, return false
    if (e1->_type != e2->_type) return FALSE;

    if (e1->_type == ExprAtom) {
        // If they are both atoms, compare the symbols
        return strcmp(e1->_symbol, e2->_symbol) == 0;
    } else {
        // If they are both lists, march through the elements
        e1 = e1->_list; e2 = e2->_list;
        while (e1 != NULL && e2 != NULL) {
            if (!expr_is_equal(e1, e2)) {
                return FALSE;
            }
            e1 = e1->_next; e2 = e2->_next;
        }
        return TRUE;
    }
}

char* expr_to_string(Expr_t expr) {
    debug(1, "expr_to_string\n");
    if (expr == NULL) return NULL;
    if (expr->_type == ExprAtom) {
        char* result = (char*)allocate_mem("expr_to_string/atom", NULL,
            sizeof(char) * (strlen(expr->_symbol) + 1));
        strcpy(result, expr->_symbol);
        debug(1, "/expr_to_string\n");
        return result;
    } else {
        // Count the number of children
        size_t child_count = 0;
        Expr_t child = expr->_list;
        while (child != NULL) {
            child_count++;
            child = child->_next;
        }

        // Store the children's strings
        char** child_strings =
            (char**)allocate_mem("expr_to_string/list/children", NULL,
            sizeof(char*) * child_count);
        child_count = 0;
        child = expr->_list;
        while (child != NULL) {
            child_strings[child_count] = expr_to_string(child);
            child_count++;
            child = child->_next;
        }

        // Calculate the total length
        size_t length = 0;
        for (size_t i = 0; i < length; i++) {
            length += strlen(child_strings[i]);
        }
        // Leave enough space for the parens (2), the spaces between the
        // children (child_count - 1) and for the terminating null (1)
        length += 2 + (child_count - 1) + 1;

        // Concatenate the children's strings into one string
        char* result = (char*)allocate_mem("expr_to_string/list/result", NULL,
            sizeof(char) * length);
        result[0] = 0;
        strcat(result, "(");
        for (size_t i = 0; i < child_count; i++) {
            strcat(result, child_strings[i]);
            if (i < child_count - 1) {
                strcat(result, " ");
            }
        }
        strcat(result, ")");

        // Free up the children's strings
        for (size_t i = 0; i < child_count; i++) {
            free_mem("expr_to_string/list/children", child_strings[i]);
        }
        free_mem("expr_to_string/list/children_ptr", child_strings);

        // Return the result
        debug(1, "expr_to_string\n");
        return result;
    }
}

void expr_free(Expr_t* expr_ptr) {
    Expr_t expr = *expr_ptr;
    if (expr == NULL) return;
    if (expr->_type == ExprAtom) {
        // expr->_next and expr->_prev should be NULL at this point
        free_mem("expr_free/symbol", expr->_symbol);
        *expr_ptr = NULL;
    } else {
        Expr_t list = expr->_list;
        while (list != NULL) {
            Expr_t dummy = list;
            list = list->_next;
            expr_free(&dummy);
        }
    }
    free_mem("expr_free/atom", expr);
}
