#include "env.h"

struct Term env_default_rules(Expr_t expr) {
    // Check if the symbol name is a number
    INDEX index = bytes_to_index(expr, SYMBOL_ID_BYTES);
    char* symbol = lookup_symbol_by_id(index, d);
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

    // If it is neither, return the symbol as an Expr_t
    return make_expr(expr);
}

// Retrieve the value of an expr from an env
struct Term env_lookup_term(EnvFrame_t frame, Expr_t expr) {
    // First we check if there is an entry for the expr in the mappings in the
    // current env
    EnvFrame_t current_frame = frame;
    while (current_frame != NULL) {
        for (INDEX i = 0; i < current_frame->entry_count; i++) {
            if (is_equal_expr(current_frame->mapping[i].expr, expr)) {
                INDEX index =
                    bytes_to_index(current_frame->mapping[i].term_id,
                    TERM_ID_BYTES);
                return current_frame->env_dict->term_dict->terms[index];
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
        // TODO cache it in the env
        struct Term t = default_rules(expr, frame->env_dict->symbol_dict);
        add_entry(frame, expr, t);
        return t;
    }
}

// char* env_lookup_symbol_by_id(Expr_t expr, EnvFrame_t frame) {
//     //
// }

// Returns the term corresponding to the longest match in the environment.
// The optional `bytes` pointer will be used to signal the amount of bytes that
// are matching - i.e. the next byte in the expression will not match.
struct Term* env_find_longest_match(EnvFrame_t frame, Expr_t expr, size_t* bytes) {
    size_t longest_match_size = 0;
    size_t current_match_size = 0;
    struct Term* result = NULL;
    EnvFrame_t current_frame = frame;
    while (current_frame != NULL) {
        for (size_t i = 0; i < current_frame->entry_count; i++) {
            current_match_size =
                match_size_bytes(current_frame->mapping[i].expr, expr);
            if (current_match_size > longest_match_size) {
                longest_match_size = current_match_size;
                size_t index =
                    bytes_to_index(current_frame->mapping[i].term_id,
                    TERM_ID_BYTES);
                result =
                    &(current_frame->env_dict->term_dict->terms[index]);
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
        frame = make_empty_frame(NULL);
        if (frame == NULL) {
            return Error;
        }
    }
    EnvFrame_t current_frame = frame;

    // Allocate space for the new entry
    struct Entry* new_mapping =
        (struct Entry*)allocate_mem("env/add_entry", current_frame->mapping,
        sizeof(struct Entry) * (current_frame->entry_count + 1));
    if (new_mapping == NULL) {
        return Error;
    }

    // Check if the term already exists
    INDEX index = 0;
    enum ErrorCode Error_code =
        get_term_index(frame->env_dict->term_dict, t, &index);
    if (Error_code != Success) {
        // If not, create it
        add_term_to_dict(frame->env_dict->term_dict, t, &index);
    }
    current_frame->mapping = new_mapping;
    current_frame->mapping[current_frame->entry_count].expr = expr;
    index_to_bytes(index,
        current_frame->mapping[current_frame->entry_count].term_id,
        TERM_ID_BYTES);
    current_frame->entry_count++;
    return Success;
}

void env_print_frame(EnvFrame_t frame) {
    debug(0, "\n\n===\nEnvironment:\n");
    size_t count = 0;
    while (frame != NULL) {
        //
        debug(0, "\n  Frame #%d:\n", count);

        char expr_buf[1024];
        char term_buf[1024];
        for (size_t i = 0; i < frame->entry_count; i++) {
            print_expr(frame->mapping[i].expr, frame, expr_buf);
            print_term(term_buf,
                get_term_by_id(frame->env_dict->term_dict,
                    bytes_to_index(frame->mapping[i].term_id, TERM_ID_BYTES)),
                frame);
            debug(0, "%s - %s", expr_buf, term_buf);
        }

        frame = frame->parent;
        count++;
    }
    debug(0, "End of environment\n===\n\n");
}

EnvFrame_t env_make_empty_frame(EnvFrame_t parent) {
    EnvFrame_t new_frame = (EnvFrame_t)allocate_mem("make_empty_frame", NULL,
        sizeof(struct _EnvFrame));
    if (new_frame != NULL) {
        new_frame->entry_count = 0;
        new_frame->mapping = NULL;
        new_frame->parent = parent;
        new_frame->env_dict = make_empty_env_dict();
    }
    return new_frame;
}

void env_free_frame(EnvFrame_t* frame_ptr) {
    EnvFrame_t frame = *frame_ptr;
    if (frame != NULL) {
        // Free the individual mappings
        for (size_t i = 0; i < frame->entry_count; i++) {
            debug(0, "remove_last_frame/mapping/entry\n");
            free_entry(frame->mapping[i]);
        }
        // Free the mapping list
        debug(0, "remove_last_frame/mapping\n");
        free_mem("remove_last_frame/mapping", frame->mapping);
        // Free the dict
        debug(0, "remove_last_frame/frame\n");
        free_mem("remove_last_frame/frame", frame);
    }
    *frame_ptr = NULL;
}

void env_free_entry(struct Entry e) {
    free_mem("env/free_entry", e.expr);
}
