#include "dict.h"

#include <stdio.h>

struct Dict make_empty_dict() {
    struct Dict d;
    d.count = 0;
    d.names = NULL;
    return d;
}

ErrorCode add_name(struct Dict* dict, size_t* dict_size, char* name,
        size_t* index) {
    debug(1, "add_name - dict_size: %llu dict.count: %llu symbol: %llu\n",
        name, *index, *dict_size, dict->count, name);
    if (*dict_size == dict->count) {
        uint8_t error_code = _grow_dict(dict, dict_size);
        if (error_code != SUCCESS) {
            error("add_name: Couldn't grow expr\n");
            debug(1, "/append_token\n");
            return error_code;
        }
    }
    *index = dict->count;
    // Copy the symbol, because it will be deleted
    char* my_symbol =
        (char*)allocate_mem("dict/add_name", NULL, sizeof(char) * (strlen(name) + 1));
    strcpy(my_symbol, name);
    dict->names[dict->count] = my_symbol;
    dict->count++;
    debug(1, "/add_name\n");
    return SUCCESS;
}

ErrorCode finalize_dict(struct Dict* dict, size_t* dict_size) {
    if (*dict_size == 0) {
        return SUCCESS;
    }
    if (dict->count < *dict_size) {
        char** new_ptr =
            (char**)allocate_mem("dict/finalize_dict", dict->names,
            dict->count * sizeof(char*));
        if (new_ptr == NULL) {
            error("finalize_dict: couldn't free up unnecessary memory\n");
            return ERROR;
        }
        dict->names = new_ptr;
        *dict_size = dict->count;
    }
    return SUCCESS;
}

void free_dict(struct Dict* dict) {
    for (size_t i = 0; i < dict->count; i++) {
        debug(1, "Deleting %llu. entry (%llu)\n", i, (size_t)dict->names[i]);
        free_mem("free_dict/name[i]", dict->names[i]);
    }
    free_mem("free_dict/names", dict->names);
    dict->names = NULL;
    dict->count = 0;
}

void print_dict(struct Dict dict, char* buf) {
    int count = sprintf(buf, "Dict: %llu entries:\n", dict.count);
    for (size_t i = 0; i < dict.count; i++) {
        count += sprintf(buf + count, "%llu. %s\n", i, dict.names[i]);
    }
}

// PRIVATE METHODS

ErrorCode _grow_dict(struct Dict* dict, size_t* dict_size) {
    debug(1, "_grow_dict(%llu -> %llu)\n",
        *dict_size, *dict_size + EXPR_BUFFER_SIZE);
    char** new_ptr =
        (char**)allocate_mem("dict/_grow_dict", dict->names,
        (*dict_size + EXPR_BUFFER_SIZE) * sizeof(char*));
    if (new_ptr == NULL) {
        error("_grow_dict: couldn't allocate more memory\n");
        return ERROR;
    }
    dict->names = new_ptr;
    *dict_size += EXPR_BUFFER_SIZE;
    return SUCCESS;
}