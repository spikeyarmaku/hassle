#include "parse.h"

Parser_t _create_parser(char* file_name) {
    debug(1, "_create_parser\n");
    Parser_t parser = (Parser_t)allocate_mem("_create_parser/parser", NULL,
        sizeof(struct Parser));
    FILE* fp = fopen(file_name, "r");

    // Get file size
    rewind(fp);
    long int start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp) - start;
    rewind(fp); // fseek(fp, 0L, SEEK_SET);

    char* file_content =
        (char*)allocate_mem("_create_parser/file_content", NULL,
        sizeof(char) * (size + 1));
    
    size_t chars_read = fread(file_content, sizeof(char), size, fp);
    
    // Add terminating null.
    // Sometimes multiple bytes are read, but one byte is reported (like on
    // Windows "\r\n" is read as "\n"). Thus, `chars_read` is used instead of
    // `size`.
    file_content[chars_read] = 0;

    fclose(fp);

    parser->stream = file_content;
    parser->counter = 0;
    debug(1, "/_create_parser\n");
    return parser;
}

void _free_parser(Parser_t* parser_ptr) {
    debug(1, "_free_parser\n");
    Parser_t parser = *parser_ptr;
    if (parser != NULL) {
        if (parser->stream != NULL) {
            free_mem("_free_parser", parser->stream);
        }
        free_mem("_free_parser", parser);
    }
    *parser_ptr = NULL;
    debug(1, "/_free_parser\n");
}

// Read the next token, and set the stream pointer to the next non-whitespace
// character
struct Token _get_next_token(ErrorCode_t* error_code, Parser_t parser) {
    debug(1, "_get_next_token\n");
    *error_code = Success;

    struct Token token;
    token.str = NULL;
    
    // Decide what type of token to read based on the first character
    char c = _get_current_char(parser);
    if (c == 0) {
        token.type = Eos;
    } else if (c == '(') {
        token.type = OpenParen;
    } else if (c == ')') {
        token.type = CloseParen;
    } else {
        _read_symbol(&token, parser);
    }
    
    // If it is a simple token, advance the stream pointer
    if (token.type != Symbol) {
        _get_next_char(parser);
    }

    // Set the stream pointer to the next non-whitespace character
    _consume_whitespace(parser);

    debug(1, "/_get_next_token\n");
    return token;
}

// Read a string (enclosed in quotes) or an identifier
ErrorCode_t _read_symbol(struct Token* token, Parser_t parser) {
    debug(1, "_read_symbol\n");
    char c = _get_current_char(parser);
    if (c == '"') {
        // String
        ErrorCode_t error_code = _read_string(token, parser);
        debug(1, "/_read_symbol\n");
        return error_code;
    } else {
        // Not a string, disallowed characters: "() and whitespace
        ErrorCode_t error_code = _read_identifier(token, parser);
        debug(1, "/_read_symbol\n");
        return error_code;
    }
    debug(1, "/_read_symbol\n");
    return Success;
}

// Read a string enclosed in quotes and trailed by at least one whitespace
// character or a closing parenthesis or EOF
ErrorCode_t _read_string(struct Token* token, Parser_t parser) {
    debug(1, "_read_string\n");
    long int starting_position = parser->counter;
    long int ending_position = parser->counter;
    int escape_counter = 0;
    char c = _get_next_char(parser);

    // Find the end position of the string
    int is_escaped = 0;
    int go_on = 1;
    while (go_on) {
        if (is_escaped == 0) {
            if (c == '"') {
                go_on = 0;
                c = _get_next_char(parser);
                ending_position = parser->counter;
            } else {
                if (c == '\\') {
                    // escape a character
                    is_escaped = 1;
                    escape_counter++;
                }
                c = _get_next_char(parser);
            }
        } else {
            c = _get_next_char(parser);
            is_escaped = 0;
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = _get_current_char(parser);
    if ((!_is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        debug(1, "/_read_string\n");
        return Error;
    }

    // Copy the string into a buffer, without the escape characters
    long int total_char_count = ending_position - starting_position;
    long int char_count = total_char_count - escape_counter;
    escape_counter = 0;
    is_escaped = 0;
    char* str = (char*)allocate_mem("parse/_read_string", NULL, sizeof(char) * (char_count + 1));
    for (long int i = 0; i < total_char_count; i++) {
        char c = parser->stream[starting_position + i];
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
    token->type = Symbol;
    token->str = str;
    debug(1, "/_read_string\n");
    return Success;
}

// Read an identifier that is either trailed by a whitespace character or a
// closing parenthesis or EOF
ErrorCode_t _read_identifier(struct Token* token, Parser_t parser) {
    debug(1, "_read_identifier\n");
    long int starting_position = parser->counter;
    long int ending_position = parser->counter;
    char c = _get_current_char(parser);

    // Find the end position of the identifier
    int go_on = 1;
    while (go_on) {
        if (c == '\\' || c == '(' || c == ')' || c == '"' || c == ';'
                || _is_whitespace(c) || c == 0) {
            // escape a character
            go_on = 0;
            ending_position = parser->counter;
        } else {
            c = _get_next_char(parser);
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = _get_current_char(parser);
    if ((!_is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        debug(1, "/_read_identifier\n");
        return Error;
    }

    // Copy the string into a buffer
    long int char_count = ending_position - starting_position;
    char* str =
        (char*)allocate_mem("parse/_read_identifier", NULL,
        sizeof(char) * (char_count + 1));
    for (long int i = 0; i < char_count; i++) {
        char c = parser->stream[starting_position + i];
        str[i] = c;
    }
    str[char_count] = 0; // Terminating null
    
    // Fill the token
    token->type = Symbol;
    token->str = str;
    debug(1, "/_read_identifier\n");
    return Success;
}

void _free_token(struct Token* token) {
    if (token == NULL) return;
    if (token->str == NULL) return;
    free_mem("_free_token/token_str", token->str);
}

char _get_next_char(Parser_t parser) {
    if (parser->stream[parser->counter] != 0) {
        parser->counter++;
    }
    return _get_current_char(parser);
}

char _get_current_char(Parser_t parser) {
    if (parser->counter >= 0) {
        return parser->stream[parser->counter];
    } else {
        return 0;
    }
}

BOOL _is_whitespace(char c) {
    return c == ' '  || c == '\n' || c == '\t'
        || c == '\v' || c == '\f' || c == '\r';
}

// Set the pointer to the next non-whitespace character
void _consume_whitespace(Parser_t parser) {
    int go_on = 1;
    char c;
    while (go_on) {
        // Read the next character
        c = _get_current_char(parser);
        // If it is not a whitespace, or it is EOF, stop
        if (!_is_whitespace(c) || c == 0) {
            return;
        } else {
            _get_next_char(parser);
        }
    }
}

Expr_t parse_from_file(ErrorCode_t* error_code, char* file_name) {
    Parser_t parser = _create_parser(file_name);

    Expr_t result = _parse(error_code, parser);
    _free_parser(&parser);
    return result;
}

Expr_t parse_from_str(ErrorCode_t* error_code, char* input) {
    Parser_t parser = (Parser_t)allocate_mem("parse_from_str", NULL,
        sizeof(struct Parser));
    parser->stream = input;
    parser->counter = 0;
    
    return _parse(error_code, parser);
}

Expr_t _parse(ErrorCode_t* error_code, Parser_t parser) {
    debug(1, "_parse (counter: %llu)\n", parser->counter);
    *error_code = Success;

    struct Token token = _get_next_token(error_code, parser);
    switch (token.type) {
        case Symbol: {
            Expr_t result = expr_make_atom(error_code, token.str);
            _free_token(&token);
            debug(1, "/_parse\n");
            return result;
        }
        case OpenParen: {
            Expr_t list = expr_make_empty_list(error_code);
            if (*error_code != Success) {
                debug(1, "/_parse\n");
                return list;
            }
            Expr_t child = _parse(error_code, parser);
            while (child != NULL) {
                expr_add_to_list(error_code, list, child);
                child = _parse(error_code, parser);
            }
            debug(1, "/_parse\n");
            return list;
        }
        case CloseParen: {
            debug(1, "/_parse\n");
            return NULL;
        }
        case Eos: {
            debug(1, "/_parse\n");
            return NULL;
        }
        default: {
            debug(1, "/_parse\n");
            return NULL;
        }
    }
}

