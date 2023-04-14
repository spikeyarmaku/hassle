#include "env.h"

struct EnvFrame {
    size_t entry_count;
    struct Entry* mapping;

    struct EnvFrame* parent;

    Stack_t stack;
};

Term_t env_default_rules(Expr_t expr) {
    debug_start("env_default_rules\n");
    if (expr_is_list(expr)) {
        Term_t result = term_make_expr(expr);
        debug_end("/env_default_rules\n");
        return result;
    }
    
    // Check if the symbol name is a number
    char* ptr = expr_get_symbol(expr);
    BOOL is_number = TRUE;
    while (*ptr != 0 && is_number == TRUE) {
        if (isdigit(*ptr) == 1 || *ptr == '.' || *ptr == '-' || *ptr == '+' ||
                *ptr == '_' || *ptr == ',') {
            ptr++;
        } else {
            is_number = FALSE;
        }
    }
    if (is_number) {
        Rational_t r = string_to_rational(expr_get_symbol(expr));
        Term_t term = term_make_number(r);
        rational_free(r);
        debug_end("/env_default_rules\n");
        return term;
    }

    // If not, check if it is a string (starts and ends with quotes - rest is
    // handled by the parser)
    ptr = expr_get_symbol(expr);
    BOOL is_string = *ptr == '"';
    while (*ptr != 0) {
        ptr++;
    }
    is_string = is_string && *(ptr-1) == '"';
    if (is_string) {
        Term_t result = term_make_string(expr_get_symbol(expr));
        debug_end("/env_default_rules\n");
        return result;
    }

    // If it is neither, return the symbol as an Expr_t
    Term_t result = term_make_expr(expr);
    debug_end("/env_default_rules\n");
    return result;
}

// Retrieve a copy of the value of an expr from an env, or return a newly
// created value
Term_t env_lookup_term(EnvFrame_t frame, Expr_t expr) {
    debug_start("env_lookup_term\n");
    // First we check if there is an entry for the expr in the mappings in the
    // current env
    EnvFrame_t current_frame = frame;
    while (current_frame != NULL) {
        for (INDEX i = 0; i < current_frame->entry_count; i++) {
            if (expr_is_equal(current_frame->mapping[i].name, expr)) {
                Term_t result = term_copy(current_frame->mapping[i].value);
                debug_end("/env_lookup_term\n");
                return result;
            }
        }
        // If not, check the parent env
        current_frame = current_frame->parent;
    }

    // If there isn't, check if it is a list, or an atom
    if (expr_is_list(expr)) {
        // If it is a list, return the list
        Term_t result = term_make_expr(expr);
        debug_end("/env_lookup_term\n");
        return result;
    } else {
        // If it is an atom, apply the rules
        // TODO cache it in the env
        Term_t result = env_default_rules(expr);
        // env_add_entry(frame, expr, t); // If you cache it here, it will get
        // cached in the ground env, which makes for a nice crash later on
        debug_end("/env_lookup_term\n");
        return result;
    }
}

// Returns the term corresponding to the longest match in the environment.
// The optional `bytes` pointer will be used to signal the amount of bytes that
// are matching - i.e. the next byte in the expression will not match.
Term_t env_find_longest_match(EnvFrame_t frame, Expr_t expr, size_t* bytes) {
    return NULL;    // currently it doesn't do anything to make testing other
                    // things easier, will fix later
    // size_t longest_match_size = 0;
    // size_t current_match_size = 0;
    // struct Term* result = NULL;
    // EnvFrame_t current_frame = frame;
    // while (current_frame != NULL) {
    //     for (size_t i = 0; i < current_frame->entry_count; i++) {
    //         current_match_size =
    //             expr_match_size(current_frame->mapping[i].name, expr);
    //         if (current_match_size > longest_match_size) {
    //             longest_match_size = current_match_size;
    //             result = current_frame->mapping[i].value;
    //         }
    //     }

    //     current_frame = current_frame->parent;
    // }
    // if (bytes != NULL) {
    //     *bytes = longest_match_size;
    // }
    // return result;
}

ErrorCode_t env_add_entry(EnvFrame_t frame, Expr_t expr, struct Term* term) {
    debug_start("env_add_entry - expr_ptr: %llu\n", (size_t)expr);
    if (frame == NULL) {
        frame = env_make_empty_frame(NULL);
        if (frame == NULL) {
            debug_end("/env_add_entry\n");
            return Error;
        }
    }
    EnvFrame_t current_frame = frame;

    // Check if the term already exists in the current frame (parents excluded)
    INDEX index = 0;
    BOOL found = FALSE;
    while (index < current_frame->entry_count && found == FALSE) {
        if (expr_is_equal(current_frame->mapping[index].name, expr)) {
            found = TRUE;
        } else {
            index++;
        }
    }
    if (found) {
        current_frame->mapping[index].value = term_copy(term);
    } else {
        // Allocate space for the new entry
        struct Entry* new_mapping =
            (struct Entry*)allocate_mem("env_add_entry", current_frame->mapping,
            sizeof(struct Entry) * (current_frame->entry_count + 1));
        if (new_mapping == NULL) {
            debug_end("/env_add_entry\n");
            return Error;
        }
        current_frame->mapping = new_mapping;
        current_frame->mapping[current_frame->entry_count].name =
            expr_copy(expr);
        current_frame->mapping[current_frame->entry_count].value =
            term_copy(term);
        current_frame->entry_count++;
    }
    
    // env_print_frame(frame);
    debug_end("/env_add_entry\n");
    return Success;
}

void env_print_frame(EnvFrame_t frame) {
    debug_start("\n===\nEnvironment:\n");
    size_t count = 0;
    while (frame != NULL) {
        debug("\n  Frame #%d:\n", count);

        for (size_t i = 0; i < frame->entry_count; i++) {
            expr_print(frame->mapping[i].name);
            printf(" - ");
            term_print(frame->mapping[i].value);
            printf("\n");
        }

        frame = frame->parent;
        count++;
    }
    debug_end("End of environment\n===\n\n");
}

EnvFrame_t env_make_empty_frame(EnvFrame_t parent) {
    EnvFrame_t new_frame = (EnvFrame_t)allocate_mem("make_empty_frame", NULL,
        sizeof(struct EnvFrame));
    if (new_frame != NULL) {
        new_frame->entry_count = 0;
        new_frame->mapping = NULL;
        new_frame->parent = parent;
        new_frame->stack = stack_make_empty();
    }
    return new_frame;
}

void env_free_frame(EnvFrame_t* frame_ptr) {
    EnvFrame_t frame = *frame_ptr;
    if (frame != NULL) {
        // Free the individual mappings
        for (size_t i = 0; i < frame->entry_count; i++) {
            debug("env_free_frame/mapping/entry\n");
            env_free_entry(frame->mapping[i]);
        }
        // Free the mapping list
        debug("env_free_frame/mapping\n");
        free_mem("env_free_frame/mapping", frame->mapping);
        // Free the dict
        debug("env_free_frame/frame\n");
        free_mem("env_free_frame/frame", frame);
        // Free the stack
        stack_free(&(frame->stack));
    }
    *frame_ptr = NULL;
}

void env_free_entry(struct Entry e) {
    expr_free(&(e.name));
    term_free(&(e.value));
}

Stack_t env_get_stack(EnvFrame_t frame) {
    return frame->stack;
}
