#include "parse.h"

struct _Parser _create_parser(char* file_name) {
    struct _Parser p;
    FILE* fp = fopen(file_name, "r");

    // Get file size
    rewind(fp);
    long int start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp) - start;
    rewind(fp); // fseek(fp, 0L, SEEK_SET);

    char* file_content = (char*)allocate_mem(NULL, sizeof(char) * (size + 1));
    
    size_t chars_read = fread(file_content, sizeof(char), size, fp);
    
    // Add terminating null.
    // Sometimes multiple bytes are read, but one byte is reported (like on
    // Windows "\r\n" is read as "\n"). Thus, `chars_read` is used instead of
    // `size`.
    file_content[chars_read] = 0;

    fclose(fp);

    p.stream = file_content;
    p.counter = 0;
    return p;
}

void _free_parser(struct _Parser* p) {
    if (p->stream != NULL) {
        free_mem(p->stream);
    }
    p->stream = NULL;
    p->counter = 0;
}

// Read the next token, and set the stream pointer to the next non-whitespace
// character
ErrorCode _get_next_token(struct _Parser* p, struct _Token* t) {
    debug(1, "_get_next_token()\n");
    t->str = NULL;
    int error = SUCCESS;
    
    // Decide what type of token to read based on the first character
    char c = _get_current_char(p);
    if (c == 0) {
        t->type = Eos;
    } else if (c == '(') {
        t->type = OpenParen;
    } else if (c == ')') {
        t->type = CloseParen;
    } else {
        error = _read_symbol(p, t);
    }
    
    // If it is a simple token, advance the stream pointer
    if (t->type != Symbol) {
        _get_next_char(p);
    }

    // Set the stream pointer to the next non-whitespace character
    _consume_whitespace(p);

    debug(1, "/_get_next_token\n");
    return error;
}

// Read a string (enclosed in quotes) or an identifier
ErrorCode _read_symbol(struct _Parser* p, struct _Token* t) {
    int error = SUCCESS;

    char c = _get_current_char(p);
    if (c == '"') {
        // String
        error = _read_string(p, t);
    } else {
        // Not a string, disallowed characters: "() and whitespace
        error = _read_identifier(p, t);
    }
    return error;
}

// Read a string enclosed in quotes and trailed by at least one whitespace
// character or a closing parenthesis or EOF
ErrorCode _read_string(struct _Parser* p, struct _Token* t) {
    long int starting_position = p->counter;
    long int ending_position = p->counter;
    int escape_counter = 0;
    char c = _get_next_char(p);

    // Find the end position of the string
    int is_escaped = 0;
    int go_on = 1;
    while (go_on) {
        if (is_escaped == 0) {
            if (c == '"') {
                go_on = 0;
                c = _get_next_char(p);
                ending_position = p->counter;
            } else {
                if (c == '\\') {
                    // escape a character
                    is_escaped = 1;
                    escape_counter++;
                }
                c = _get_next_char(p);
            }
        } else {
            c = _get_next_char(p);
            is_escaped = 0;
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = _get_current_char(p);
    if ((!_is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        return ERROR;
    }

    // Copy the string into a buffer, without the escape characters
    long int total_char_count = ending_position - starting_position;
    long int char_count = total_char_count - escape_counter;
    escape_counter = 0;
    is_escaped = 0;
    char* str = (char*)allocate_mem(NULL, sizeof(char) * (char_count + 1));
    for (long int i = 0; i < total_char_count; i++) {
        char c = p->stream[starting_position + i];
        if (is_escaped) {
            str[i - escape_counter] = c;
            is_escaped = 0;
        } else {
            if (c == '\\') {
                is_escaped = 1;
                escape_counter++;
            } else {
                str[i - escape_counter] = c;
            }
        }
    }
    str[char_count] = 0; // Terminating null

    // Fill the token
    t->type = Symbol;
    t->str = str;
    return SUCCESS;
}

// Read an identifier that is either trailed by a whitespace character or a
// closing parenthesis or EOF
ErrorCode _read_identifier(struct _Parser* p, struct _Token* t) {
    long int starting_position = p->counter;
    long int ending_position = p->counter;
    char c = _get_current_char(p);

    // Find the end position of the identifier
    int go_on = 1;
    while (go_on) {
        if (c == '\\' || c == '(' || c == ')' || c == '"' || c == ';'
                || _is_whitespace(c) || c == 0) {
            // escape a character
            go_on = 0;
            ending_position = p->counter;
        } else {
            c = _get_next_char(p);
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = _get_current_char(p);
    if ((!_is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        return ERROR;
    }

    // Copy the string into a buffer
    long int char_count = ending_position - starting_position;
    char* str = (char*)allocate_mem(NULL, sizeof(char) * (char_count + 1));
    for (long int i = 0; i < char_count; i++) {
        char c = p->stream[starting_position + i];
        str[i] = c;
    }
    str[char_count] = 0; // Terminating null
    
    // Fill the token
    t->type = Symbol;
    t->str = str;
    return SUCCESS;
}

void _free_token(struct _Token* t) {
    if (t->str != NULL) {
        free_mem(t->str);
    }
    t->str = NULL;
}

char _get_next_char(struct _Parser* p) {
    if (p->stream[p->counter] != 0) {
        p->counter++;
    }
    return _get_current_char(p);
}

char _get_current_char(struct _Parser* p) {
    if (p->counter >= 0) {
        return p->stream[p->counter];
    } else {
        return 0;
    }
}

BOOL _is_whitespace(char c) {
    return c == ' '  || c == '\n' || c == '\t'
        || c == '\v' || c == '\f' || c == '\r';
}

// Set the pointer to the next non-whitespace character
void _consume_whitespace(struct _Parser* p) {
    int go_on = 1;
    char c;
    while (go_on) {
        // Read the next character
        c = _get_current_char(p);
        // If it is not a whitespace, or it is EOF, stop
        if (!_is_whitespace(c) || c == 0) {
            return;
        } else {
            _get_next_char(p);
        }
    }
}

ErrorCode parse_from_file(char* file_name, Expr* result_expr,
        struct Dict* result_dict) {
    struct _Parser parser = _create_parser(file_name);

    ErrorCode error_code = _parse(parser, result_expr, result_dict);
    _free_parser(&parser);
    return error_code;
}

ErrorCode parse_from_str(char* input, Expr* result_expr,
        struct Dict* result_dict) {
    struct _Parser parser;
    parser.stream = input;
    parser.counter = 0;
    
    return _parse(parser, result_expr, result_dict);
}

ErrorCode _parse(struct _Parser parser, Expr* result_expr,
        struct Dict* result_dict) {
    debug(1, "_parse()\n");
    struct ExprBuilder b = make_expr_builder(result_dict);
    struct _Token t;

    int go_on = 1;
    while (go_on) {
        if (_get_next_token(&parser, &t)) {
            error("_parse: error reading next token\n");
            break;
        }

        uint8_t error_code = SUCCESS;
        if (t.type == Symbol) {
            error_code = append_token(&b, t.type, t.str);
        } else {
            error_code = append_token(&b, t.type, NULL);
        }
        if (error_code != SUCCESS) {
            error("Error while appending token (%d)\n", t.type);
            return error_code;
        }
        _free_token(&t);
        t.str = NULL;

        // If we are at the end of the stream, end the loop
        // (At this point, the Eos token should already be added to the expr)
        if (t.type == Eos) {
            go_on = 0;
        }
    }
    finalize_builder(&b);
    *result_expr = b.expr;
    *result_dict = b.dict;
    return SUCCESS;
}

