#include "env.h"

struct Term env_default_rules(Expr_t expr) {
    if (expr_is_list(expr)) {
        return term_make_expr(expr);
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
        return term_make_number(string_to_rational(expr_get_symbol(expr)));
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
        return term_make_string(expr_get_symbol(expr));
    }

    // If it is neither, return the symbol as an Expr_t
    return term_make_expr(expr);
}

// Retrieve the value of an expr from an env
struct Term env_lookup_term(EnvFrame_t frame, Expr_t expr) {
    // First we check if there is an entry for the expr in the mappings in the
    // current env
    EnvFrame_t current_frame = frame;
    while (current_frame != NULL) {
        for (INDEX i = 0; i < current_frame->entry_count; i++) {
            if (expr_is_equal(current_frame->mapping[i].name, expr)) {
                return current_frame->mapping[i].value;
            }
        }
        // If not, check the parent env
        current_frame = current_frame->parent;
    }

    // If there isn't, check if it is a list, or an atom
    if (expr_is_list(expr)) {
        // If it is a list, return the list
        return term_make_expr(expr);
    } else {
        // If it is an atom, apply the rules
        // TODO cache it in the env
        struct Term t = env_default_rules(expr);
        // env_add_entry(frame, expr, t); // If you cache it here, it will get
        // cached in the ground env, which makes for a nice crash later on
        return t;
    }
}

// Returns the term corresponding to the longest match in the environment.
// The optional `bytes` pointer will be used to signal the amount of bytes that
// are matching - i.e. the next byte in the expression will not match.
struct Term* env_find_longest_match(EnvFrame_t frame, Expr_t expr,
        size_t* bytes) {
    size_t longest_match_size = 0;
    size_t current_match_size = 0;
    struct Term* result = NULL;
    EnvFrame_t current_frame = frame;
    while (current_frame != NULL) {
        for (size_t i = 0; i < current_frame->entry_count; i++) {
            current_match_size =
                expr_match_size(current_frame->mapping[i].name, expr);
            if (current_match_size > longest_match_size) {
                longest_match_size = current_match_size;
                result = &(current_frame->mapping[i].value);
            }
        }

        current_frame = current_frame->parent;
    }
    if (bytes != NULL) {
        *bytes = longest_match_size;
    }
    return result;
}

ErrorCode_t env_add_entry(EnvFrame_t frame, Expr_t expr, struct Term t) {
    if (frame == NULL) {
        frame = env_make_empty_frame(NULL);
        if (frame == NULL) {
            return Error;
        }
    }
    EnvFrame_t current_frame = frame;

    // Check if the term already exists
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
        current_frame->mapping[index].value = t;
    } else {
        // Allocate space for the new entry
        struct Entry* new_mapping =
            (struct Entry*)allocate_mem("env/add_entry", current_frame->mapping,
            sizeof(struct Entry) * (current_frame->entry_count + 1));
        if (new_mapping == NULL) {
            return Error;
        }
        current_frame->mapping = new_mapping;
        current_frame->mapping[current_frame->entry_count].name =
            expr_copy(expr);
        current_frame->mapping[current_frame->entry_count].value = t;
        current_frame->entry_count++;
    }
    return Success;
}

void env_print_frame(EnvFrame_t frame) {
    debug(1, "\n===\nEnvironment:\n");
    size_t count = 0;
    while (frame != NULL) {
        debug(0, "\n  Frame #%d:\n", count);

        for (size_t i = 0; i < frame->entry_count; i++) {
            expr_print(frame->mapping[i].name);
            printf(" - ");
            term_print(frame->mapping[i].value);
            printf("\n");
        }

        frame = frame->parent;
        count++;
    }
    debug(-1, "End of environment\n===\n\n");
}

EnvFrame_t env_make_empty_frame(EnvFrame_t parent) {
    EnvFrame_t new_frame = (EnvFrame_t)allocate_mem("make_empty_frame", NULL,
        sizeof(struct EnvFrame));
    if (new_frame != NULL) {
        new_frame->entry_count = 0;
        new_frame->mapping = NULL;
        new_frame->parent = parent;
    }
    return new_frame;
}

void env_free_frame(EnvFrame_t* frame_ptr) {
    EnvFrame_t frame = *frame_ptr;
    if (frame != NULL) {
        // Free the individual mappings
        for (size_t i = 0; i < frame->entry_count; i++) {
            debug(0, "env_free_frame/mapping/entry - ");
            expr_print(frame->mapping[i].name); debug(0, "\n");
            env_free_entry(frame->mapping[i]);
        }
        // Free the mapping list
        debug(0, "env_free_frame/mapping\n");
        free_mem("env_free_frame/mapping", frame->mapping);
        // Free the dict
        debug(0, "env_free_frame/frame\n");
        free_mem("env_free_frame/frame", frame);
    }
    *frame_ptr = NULL;
}

void env_free_entry(struct Entry e) {
    expr_free(&(e.name));
    term_free(e.value);
}
