#include "env.h"

struct Term default_rules(struct Expr expr) {
    // Check if the symbol name is a number
    char* ptr = expr.symbol;
    uint8_t is_number = 1;
    while (*ptr != 0 && is_number == 1) {
        if (isdigit(*ptr) == 0 || *ptr == '.' || *ptr == '-' || *ptr == '+' ||
                *ptr == '_' || *ptr == ',') {
            ptr++;
        } else {
            is_number = 0;
        }
    }
    if (is_number) {
        return make_number(string_to_rational(expr.symbol));
    }

    // If not, check if it is a string (starts and ends with quotes - rest is
    // handled by the parser)
    ptr = expr.symbol;
    uint8_t is_string = *ptr == '"';
    while (*ptr != 0) {
        ptr++;
    }
    is_string = is_string && *(ptr-1) == '"';
    if (is_string) {
        return make_string(expr.symbol);
    }

    // If it is neither, return the symbol as an Expr
    return make_expr(expr);
}

// Retrieve the value of an expr from an env
struct Term env_lookup(struct Env* env, struct Expr expr) {
    // First we check if there is an entry for the expr in the mappings in the
    // current env
    while (env != NULL) {
        struct Entry* entry = env->mapping;
        while (entry != NULL) {
            if (is_equal_expr(entry->expr, expr)) {
                return entry->term;
            }
            entry = entry->next;
        }
        // If not, check the parent env
        env = env->parent;
    }

    // If there isn't, apply the rules
    return default_rules(expr);
}

struct Term* find_longest_match(struct Env* env, struct Expr expr, size_t* size)
{
    size_t longest_match_size = 0;
    size_t current_match_size = 0;
    struct Term* result = NULL;
    while (env != NULL) {
        struct Entry* current = env->mapping;
        while (current != NULL) {
            current_match_size = match_size(current->expr, expr);
            if (current_match_size > longest_match_size) {
                longest_match_size = current_match_size;
                result = &(current->term);
            }

            current = current->next;
        }

        env = env->parent;
    }
    if (size != NULL) {
        *size = longest_match_size;
    }
    return result;
}
