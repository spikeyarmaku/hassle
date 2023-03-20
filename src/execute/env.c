#include "env.h"

Env make_empty_env() {
    Env env = (Env)allocate_mem("make_empty_env", NULL, sizeof(struct _Env));
    env->current_frame = NULL;
    env->dict = make_empty_dict();
    return env;
}

void free_env(Env env) {
    free_dict(&(env->dict));
    while (env->current_frame != NULL) {
        remove_last_frame(env);
    }

    free_mem("free_env", env);
}

struct Term default_rules(Expr expr, struct Dict d) {
    // Check if the symbol name is a number
    char* symbol = lookup_symbol_by_id(expr, d);
    char* ptr = symbol;
    uint8_t is_number = 1;
    while (*ptr != 0 && is_number == 1) {
        if (isdigit(*ptr) == 1 || *ptr == '.' || *ptr == '-' || *ptr == '+' ||
                *ptr == '_' || *ptr == ',') {
            ptr++;
        } else {
            is_number = 0;
        }
    }
    if (is_number) {
        return make_number(string_to_rational(symbol));
    }

    // If not, check if it is a string (starts and ends with quotes - rest is
    // handled by the parser)
    ptr = symbol;
    uint8_t is_string = *ptr == '"';
    while (*ptr != 0) {
        ptr++;
    }
    is_string = is_string && *(ptr-1) == '"';
    if (is_string) {
        return make_string(symbol);
    }

    // If it is neither, return the symbol as an Expr
    return make_expr(expr);
}

// Retrieve the value of an expr from an env
struct Term env_lookup(Env env, Expr expr) {
    // First we check if there is an entry for the expr in the mappings in the
    // current env
    EnvFrame current_frame = env->current_frame;
    while (current_frame != NULL) {
        for (size_t i = 0; i < current_frame->entry_count; i++) {
            if (is_equal_expr(current_frame->mapping[i].expr, expr)) {
                return current_frame->mapping[i].term;
            }
        }
        // If not, check the parent env
        current_frame = current_frame->parent;
    }

    // If there isn't, check if it is a list, or an atom
    if (expr[0] == OpenParen) {
        // If it is a list, return the list
        struct Term t;
        t.type = ExprTerm;
        t.expr = expr;
        return t;
    } else {
        // If it is an atom, apply the rules
        return default_rules(expr, env->dict);
    }
}

// Returns the term corresponding to the longest match in the environment.
// The optional `bytes` pointer will be used to signal the amount of bytes that
// are matching - i.e. the next byte in the expression will not match.
struct Term* find_longest_match(Env env, Expr expr, size_t* bytes) {
    size_t longest_match_size = 0;
    size_t current_match_size = 0;
    struct Term* result = NULL;
    EnvFrame current_frame = env->current_frame;
    while (current_frame != NULL) {
        for (size_t i = 0; i < current_frame->entry_count; i++) {
            current_match_size =
                match_size_bytes(current_frame->mapping[i].expr, expr);
            if (current_match_size > longest_match_size) {
                longest_match_size = current_match_size;
                result = &(current_frame->mapping[i].term);
            }
        }

        current_frame = current_frame->parent;
    }
    if (bytes != NULL) {
        *bytes = longest_match_size;
    }
    return result;
}

ErrorCode add_entry(Env env, Expr expr, struct Term t) {
    if (env->current_frame == NULL) {
        ErrorCode error_code = add_empty_frame(env);
        if (error_code != SUCCESS) {
            return error_code;
        }
    }
    EnvFrame current_frame = env->current_frame;
    struct Entry* new_mapping =
        (struct Entry*)allocate_mem("env/add_entry", current_frame->mapping,
        sizeof(struct Entry) * (current_frame->entry_count + 1));
    if (new_mapping == NULL) {
        return ERROR;
    }
    current_frame->mapping = new_mapping;
    current_frame->mapping[current_frame->entry_count].expr = expr;
    current_frame->mapping[current_frame->entry_count].term = t;
    current_frame->entry_count++;
    return SUCCESS;
}

ErrorCode add_empty_frame(Env env) {
    if (env == NULL) {
        return ERROR;
    }

    EnvFrame new_frame = (EnvFrame)allocate_mem("env/add_empty_frame", NULL, sizeof(struct _EnvFrame));
    if (new_frame == NULL) {
        return ERROR;
    }
    new_frame->entry_count = 0;
    new_frame->mapping = NULL;
    new_frame->parent = env->current_frame;
    env->current_frame = new_frame;
    
    return SUCCESS;
}

void free_entry(struct Entry e) {
    free_expr(&(e.expr));
    free_term(e.term);
}

void remove_last_frame(Env env) {
    if (env != NULL) {
        EnvFrame current_frame = env->current_frame;
        if (current_frame != NULL) {
            // Free the individual mappings
            for (size_t i = 0; i < current_frame->entry_count; i++) {
                free_entry(current_frame->mapping[i]);
            }
            // Free the mapping list
            free_mem("remove_last_frame/mapping", current_frame->mapping);
            env->current_frame = current_frame->parent;
            // Free the frame
            free_mem("remove_last_frame/frame", current_frame);
        }
    }
}
