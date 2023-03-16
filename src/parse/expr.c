#include "expr.h"

#include <stdio.h>

struct ExprBuilder make_expr_builder(struct Dict* d) {
    struct ExprBuilder builder;
    builder.expr = NULL;
    if (d == NULL) {
        builder.dict.count = 0;
        builder.dict.names = NULL;
    } else {
        builder.dict = *d;
    }

    builder._expr_size = 0;
    builder._expr_cursor = 0;
    builder._dict_size = d == NULL ? 0 : d->count;
    return builder;
}

// Allocate more space for the expression
ErrorCode _grow_expr(struct ExprBuilder* b) {
    debug(1, "grow_expr(%llu -> %llu)", b->_expr_size, b->_expr_size + EXPR_BUFFER_SIZE);
    Expr new_ptr =
        (Expr)allocate_mem(b->expr,
        (b->_expr_size + EXPR_BUFFER_SIZE) * sizeof(uint8_t));
    if (new_ptr == NULL) {
        error("grow_expr: couldn't allocate more memory\n");
        return ERROR;
    }
    b->expr = new_ptr;
    b->_expr_size += EXPR_BUFFER_SIZE;
    return SUCCESS;
}

// Calculate the final size of the expression, and free up unnecessary allocated
// space
ErrorCode _finalize_expr(struct ExprBuilder* b) {
    if (b->_expr_size == 0) {
        return SUCCESS;
    }
    size_t new_size = 0;
    uint8_t current = b->expr[new_size];
    while (current != Eos) {
        current = b->expr[new_size];
        new_size++;
        if (current == Symbol) {
            new_size += SYMBOL_ID_BYTES;
        }
    }
    if (b->_expr_size < new_size) {
        uint8_t* new_ptr =
            (uint8_t*)allocate_mem(b->expr, new_size * sizeof(uint8_t));
        if (new_ptr == NULL) {
            error("finalize_expr: couldn't free up unnecessary memory\n");
            return ERROR;
        }
        b->expr = new_ptr;
    }
    return SUCCESS;
}

// Calculate the final size of the expr and the dict, and free up unnecessary
// allocated space
ErrorCode finalize_builder(struct ExprBuilder* b) {
    ErrorCode error_code = _finalize_expr(b);
    if (error_code == SUCCESS) {
        return finalize_dict(&(b->dict), &(b->_dict_size));
    } else {
        return error_code;
    }
}

// Return the ID of a symbol in the symbol dictionary
ErrorCode find_symbol(struct ExprBuilder* b, char* symbol, size_t* index) {
    for (size_t i = 0; i < b->dict.count; i++) {
        if (strcmp(symbol, b->dict.names[i]) == 0) {
            *index = i;
            return SUCCESS;
        }
    }
    // error("find_symbol: couldn't find symbol %s\n", symbol);
    return ERROR;
}

// Add a new token to the expr
ErrorCode append_token(struct ExprBuilder* b, uint8_t type, char* symbol) {
    debug(1, "append_token(*, %d, %s)\n", type, symbol);
    // Check if expr needs more space
    if (b == NULL) {
        error("append_token: expr builder is NULL\n");
        debug(1, "/append_token\n");
        return ERROR;
    }
    if (b->_expr_cursor + SYMBOL_ID_BYTES + 1 > b->_expr_size) {
        uint8_t error_code = _grow_expr(b);
        if (error_code != SUCCESS) {
            printf("Error while growing expr\n");
            debug(1, "/append_token\n");
            return error_code;
        }
    }
    b->expr[b->_expr_cursor] = type;
    if (type == Symbol) {
        size_t index = 0;
        if (find_symbol(b, symbol, &index) != SUCCESS) {
            uint8_t error_code =
                add_name(&(b->dict), &(b->_dict_size), symbol, &index);
            if (error_code != SUCCESS) {
                error("append_token: couldn't append symbol %s\n", symbol);
                debug(1, "/append_token\n");
                return error_code;
            }
        }
        // Store the symbol ID in little endian format
        for (uint8_t i = 0; i < SYMBOL_ID_BYTES; i++) {
            b->_expr_cursor++;
            b->expr[b->_expr_cursor] = (index & (255 << (8 * i))) >> (8 * i);
        }
    }
    b->_expr_cursor++;
    debug(1, "/append_token\n");
    return SUCCESS;
}

// Restore an index from bytes
size_t _bytes_to_index(Expr expr, size_t cursor) {
    size_t index = 0;
    size_t multiplier = 1;
    for (uint8_t i = 0; i < SYMBOL_ID_BYTES; i++) {
        index += expr[cursor] * multiplier;
        multiplier <<= 8;
        cursor++;
    }
    return index;
}

// Retrieve a symbol from an ID
char* lookup_symbol_by_id(Expr expr, size_t cursor, struct Dict* d) {
    return d->names[_bytes_to_index(expr, cursor)];
}

void free_expr(Expr* expr) {
    free_mem(*expr);
    *expr = NULL;
}

// Check if two expressions are equal by comparing their bytes.
// Return 1 if equal, 0 if not.
BOOL is_equal_expr(Expr e1, Expr e2) {
    uint8_t b1 = e1[0];
    uint8_t b2 = e2[0];

    // Check if they are valid epxressions
    if (b1 == Eos || b1 == CloseParen || b2 == Eos || b2 == CloseParen) {
        return b1 == b2;
    }

    // Check if they are symbols
    if (b1 == b2) {
        if (b1 == Symbol) {
            // Compare just one symbol
            for (size_t i = 1; i < SYMBOL_ID_BYTES + 1; i++) {
                if (e1[i] != e2[i]) {
                    return FALSE;
                }
            }
            return TRUE;
        } else {
            // Compare until the end of list (b1 and b2 are OpenParen)
            size_t counter = 1;
            enum TokenType token_type;
            uint8_t new_token_in = 0;
            uint8_t depth = 1;
            while (depth != 0) {
                b1 = e1[counter];
                b2 = e2[counter];
                
                if (b1 != b2) {
                    return FALSE;
                }

                if (new_token_in == 0) {
                    token_type = b1;
                    switch (token_type) {
                        case Symbol: {
                            new_token_in = SYMBOL_ID_BYTES;
                            break;
                        }
                        case OpenParen: {
                            depth++;
                            break;
                        }
                        case CloseParen: {
                            depth--;
                            break;
                        }
                        case Eos: {
                            depth = 0;
                            break;
                        }
                    }
                } else {
                    new_token_in--;
                }
                
                counter++;
            }
            return TRUE;
        }
    } else {
        return FALSE;
    }
}

// Compare two lists, and return the number of equal elements up until the first
// difference. E.g. comparing (a b (x y) e) and (a b (x z) e) should yield 2.
size_t match_size(Expr e1, Expr e2) {
    size_t  cursor          = 0;
    size_t  expr_count      = 0;
    uint8_t new_token_in    = 0; // How many bytes until a new token starts?
    uint8_t depth           = 0;
    uint8_t token_type      = e1[cursor];
    
    // Check if we are comparing lists
    if (e1[cursor] != OpenParen || e2[cursor] != OpenParen) {
        return 0;
    }

    while (e1[cursor] == e2[cursor] && token_type != Eos) {
        if (new_token_in == 0) {
            token_type = e1[cursor];
            if (e1[cursor] == Symbol) {
                new_token_in = SYMBOL_ID_BYTES;
                if (depth == 1) { 
                    expr_count++;
                }
            } else if (e1[cursor] == OpenParen) {
                depth++;
            } else if (e1[cursor] == CloseParen) {
                depth--;
            }
        } else {
            new_token_in--;
        }

        cursor++;
    }
    return expr_count;
}

// Compare two lists, and return the number of equal bytes up until the first
// difference. E.g. comparing (a b (x y) e) and (a b (x z) e) should yield 17.
size_t match_size_bytes(Expr e1, Expr e2) {
    size_t  cursor          = 0;
    uint8_t new_token_in    = 0; // How many bytes until a new token starts?
    uint8_t depth           = 0;
    uint8_t token_type      = e1[cursor];
    
    // Check if we are comparing lists
    if (e1[cursor] != OpenParen || e2[cursor] != OpenParen) {
        return 0;
    }

    while (e1[cursor] == e2[cursor] && token_type != Eos) {
        if (new_token_in == 0) {
            token_type = e1[cursor];
            if (e1[cursor] == Symbol) {
                new_token_in = SYMBOL_ID_BYTES;
            } else if (e1[cursor] == OpenParen) {
                depth++;
            } else if (e1[cursor] == CloseParen) {
                depth--;
            }
        } else {
            new_token_in--;
        }

        cursor++;
    }
    return cursor;
}

// Prints a human readable form of the expression into a given buffer
void print_expr(Expr expr, struct Dict* d, char* msg) {
    size_t msg_cursor = 0;
    size_t cursor = 0;
    uint8_t type = expr[cursor];
    uint8_t need_space = 0;
    while (type != Eos) {
        type = expr[cursor];
        cursor++;

        switch(type) {
            case OpenParen: {
                if (need_space == 1) {
                    msg[msg_cursor] = ' ';
                    msg_cursor++;
                    need_space = 0;
                }
                msg[msg_cursor] = '(';
                msg_cursor++;
                break;
            }
            case CloseParen: {
                msg[msg_cursor] = ')';
                msg_cursor++;
                need_space = 1;
                break;
            }
            case Eos: {
                break;
            }
            case Symbol: {
                if (need_space == 1) {
                    msg[msg_cursor] = ' ';
                    msg_cursor++;
                }
                if (d->count > 0) {
                    char* symbol = lookup_symbol_by_id(expr, cursor, d);
                    msg_cursor += sprintf(msg + msg_cursor, "%s", symbol);
                } else {
                    msg_cursor +=
                        sprintf(msg + msg_cursor, "%llu",
                        _bytes_to_index(expr, cursor));
                }
                cursor += SYMBOL_ID_BYTES;
                need_space = 1;
                break;
            }
        }
    }
    msg[msg_cursor] = 0;
}

uint8_t is_list(Expr expr) {
    return expr[0] == OpenParen;
}

uint8_t is_empty_list(Expr expr) {
    return expr[0] == OpenParen && expr[1] == CloseParen && expr[2] == Eos;
}
