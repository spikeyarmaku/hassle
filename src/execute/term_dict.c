#include "term_dict.h"

TermDict_t make_empty_term_dict() {
    TermDict_t d =
        (TermDict_t)allocate_mem("make_empty_symbol_dict", NULL,
        sizeof(struct _TermDict));
    d->count = 0;
    d->terms = NULL;
    d->capacity = 0;
    return d;
}

enum ErrorCode get_term_index(TermDict_t dict, struct Term t, INDEX* index) {
    for (INDEX i = 0; i < dict->count; i++) {
        struct Term hit = dict->terms[i];
        if (is_equal_term(t, hit)) {
            *index = i;
            return Success;
        }
    }
    return Error;
}

struct Term get_term_by_id(TermDict_t dict, INDEX i) {
    return dict->terms[i];
}

enum ErrorCode add_term_to_dict(TermDict_t dict, struct Term t,
        size_t* index) {
    debug(1, "add_term_to_dict - dict_size: %llu dict.count: %llu\n",
        dict->capacity, dict->count);
    if (dict->capacity == dict->count) {
        uint8_t Error_code = _grow_term_dict(dict);
        if (Error_code != Success) {
            Error("add_term_to_dict: Couldn't grow expr\n");
            return Error_code;
        }
    }
    *index = dict->count;
    // Copy the symbol, because it will be deleted
    dict->terms[dict->count] = t;
    dict->count++;
    debug(1, "/add_term_to_dict\n");
    return Success;
}

enum ErrorCode strip_term_dict(TermDict_t dict) {
    if (dict->capacity == 0) {
        return Success;
    }
    if (dict->count < dict->capacity) {
        struct Term* new_ptr =
            (struct Term*)allocate_mem("dict/strip_symbol_dict", dict->terms,
            dict->count * sizeof(struct Term));
        if (new_ptr == NULL) {
            Error("strip_symbol_dict: couldn't free up unnecessary memory\n");
            return Error;
        }
        dict->terms = new_ptr;
        dict->capacity = dict->count;
    }
    return Success;
}

void free_term_dict(TermDict_t* dict_ptr) {
    TermDict_t dict = *dict_ptr;
    for (size_t i = 0; i < dict->count; i++) {
        // free_mem("free_term_dict/name[i]", dict->terms[i]);
        free_term(dict->terms[i]);
    }
    free_mem("free_symbol_dict/names", dict->terms);
    free_mem("free_term_dict", dict);
    *dict_ptr = NULL;
}

// Adds more memory to the dict.
enum ErrorCode _grow_term_dict(TermDict_t dict) {
    debug(1, "_grow_term_dict(%llu -> %llu)\n",
        dict->capacity, dict->capacity + DICT_BUFFER_SIZE);
    struct Term* new_ptr =
        (struct Term*)allocate_mem("dict/_grow_symbol_dict", dict->terms,
        (dict->capacity + DICT_BUFFER_SIZE) * sizeof(struct Term));
    if (new_ptr == NULL) {
        Error("_grow_symbol_dict: couldn't allocate more memory\n");
        return Error;
    }
    dict->terms = new_ptr;
    dict->capacity += DICT_BUFFER_SIZE;
    return Success;
}