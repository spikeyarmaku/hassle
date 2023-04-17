#include "parse.h"

struct Token {
    enum TokenType type;
    char* str;
};

struct Parser {
    char* stream;
    BOOL can_free_stream;
    long int counter;
};

Parser_t        create_parser      (char*);
void            free_parser        (Parser_t*);

struct Token    get_next_token     (Parser_t);
struct Token    read_symbol        (Parser_t);
struct Token    read_string        (Parser_t);
struct Token    read_identifier    (Parser_t);
void            free_token         (struct Token);

char            get_next_char      (Parser_t);
char            get_current_char   (Parser_t);

BOOL            is_whitespace      (char);
void            consume_whitespace (Parser_t);

Expr_t          parse              (Parser_t);

Parser_t create_parser(char* file_name) {
    debug_start("create_parser\n");
    Parser_t parser = (Parser_t)allocate_mem("create_parser/parser", NULL,
        sizeof(struct Parser));
    FILE* fp = fopen(file_name, "r");

    // Get file size
    rewind(fp);
    long int start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp) - start;
    rewind(fp); // fseek(fp, 0L, SEEK_SET);

    char* file_content =
        (char*)allocate_mem("create_parser/file_content", NULL,
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
    parser->can_free_stream = TRUE;
    debug_end("/create_parser\n");
    return parser;
}

void free_parser(Parser_t* parser_ptr) {
    debug_start("free_parser\n");
    Parser_t parser = *parser_ptr;
    if (parser != NULL) {
        if (parser->stream != NULL && parser->can_free_stream) {
            free_mem("free_parser", parser->stream);
        }
        free_mem("free_parser", parser);
    }
    *parser_ptr = NULL;
    debug_end("/free_parser\n");
}

// Read the next token, and set the stream pointer to the next non-whitespace
// character
struct Token get_next_token(Parser_t parser) {
    debug_start("get_next_token\n");

    struct Token token;
    token.str = NULL;
    
    // Decide what type of token to read based on the first character
    char c = get_current_char(parser);
    if (c == 0) {
        token.type = Eos;
    } else if (c == '(') {
        token.type = OpenParen;
    } else if (c == ')') {
        token.type = CloseParen;
    } else {
        token = read_symbol(parser);
    }
    
    // If it is a simple token, advance the stream pointer
    if (token.type != Symbol) {
        get_next_char(parser);
    }

    // Set the stream pointer to the next non-whitespace character
    consume_whitespace(parser);

    debug_end("/get_next_token\n");
    return token;
}

// Read a string (enclosed in quotes) or an identifier
struct Token read_symbol(Parser_t parser) {
    debug_start("read_symbol\n");
    char c = get_current_char(parser);
    if (c == '"') {
        // String
        struct Token token = read_string(parser);
        debug_end("/read_symbol\n");
        return token;
    } else {
        // Not a string, disallowed characters: "() and whitespace
        struct Token token = read_identifier(parser);
        debug_end("/read_symbol\n");
        return token;
    }
}

// Read a string enclosed in quotes and trailed by at least one whitespace
// character or a closing parenthesis or EOF
struct Token read_string(Parser_t parser) {
    debug_start("read_string\n");
    long int starting_position = parser->counter;
    long int ending_position = parser->counter;
    int escape_counter = 0;
    char c = get_next_char(parser);

    // Find the end position of the string
    int is_escaped = 0;
    int go_on = 1;
    while (go_on) {
        if (is_escaped == 0) {
            if (c == '"') {
                go_on = 0;
                c = get_next_char(parser);
                ending_position = parser->counter;
            } else {
                if (c == '\\') {
                    // escape a character
                    is_escaped = 1;
                    escape_counter++;
                }
                c = get_next_char(parser);
            }
        } else {
            c = get_next_char(parser);
            is_escaped = 0;
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = get_current_char(parser);
    assert(is_whitespace(c) || c == ')' || c == 0);
    assert(ending_position != starting_position);

    // Copy the string into a buffer, without the escape characters
    long int total_char_count = ending_position - starting_position;
    long int char_count = total_char_count - escape_counter;
    escape_counter = 0;
    is_escaped = 0;
    char* str = (char*)allocate_mem("parse/read_string", NULL,
        sizeof(char) * (char_count + 1));
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
    struct Token token;
    token.type = Symbol;
    token.str = str;
    debug_end("/read_string\n");
    return token;
}

// Read an identifier that is either trailed by a whitespace character or a
// closing parenthesis or EOF
struct Token read_identifier(Parser_t parser) {
    debug_start("read_identifier\n");
    long int starting_position = parser->counter;
    long int ending_position = parser->counter;
    char c = get_current_char(parser);

    // Find the end position of the identifier
    int go_on = 1;
    while (go_on) {
        if (c == '\\' || c == '(' || c == ')' || c == '"' || c == ';'
                || is_whitespace(c) || c == 0) {
            // escape a character
            go_on = 0;
            ending_position = parser->counter;
        } else {
            c = get_next_char(parser);
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = get_current_char(parser);
    assert(is_whitespace(c) || c == ')' || c == 0);
    assert(ending_position != starting_position);

    // Copy the string into a buffer
    long int char_count = ending_position - starting_position;
    char* str =
        (char*)allocate_mem("parse/read_identifier", NULL,
        sizeof(char) * (char_count + 1));
    for (long int i = 0; i < char_count; i++) {
        char c = parser->stream[starting_position + i];
        str[i] = c;
    }
    str[char_count] = 0; // Terminating null
    
    // Fill the token
    struct Token token;
    token.type = Symbol;
    token.str = str;
    debug_end("/read_identifier\n");
    return token;
}

void free_token(struct Token token) {
    if (token.str == NULL) return;
    free_mem("free_token/token_str", token.str);
}

char get_next_char(Parser_t parser) {
    if (parser->stream[parser->counter] != 0) {
        parser->counter++;
    }
    return get_current_char(parser);
}

char get_current_char(Parser_t parser) {
    if (parser->counter >= 0) {
        return parser->stream[parser->counter];
    } else {
        return 0;
    }
}

BOOL is_whitespace(char c) {
    return c == ' '  || c == '\n' || c == '\t'
        || c == '\v' || c == '\f' || c == '\r';
}

// Set the pointer to the next non-whitespace character
void consume_whitespace(Parser_t parser) {
    int go_on = 1;
    char c;
    while (go_on) {
        // Read the next character
        c = get_current_char(parser);
        // If it is not a whitespace, or it is EOF, stop
        if (!is_whitespace(c) || c == 0) {
            return;
        } else {
            get_next_char(parser);
        }
    }
}

Expr_t parse_from_file(char* file_name) {
    debug_start("parse_from_file\n");
    Parser_t parser = create_parser(file_name);

    Expr_t result = parse(parser);
    free_parser(&parser);
    debug_end("/parse_from_file\n");
    return result;
}

Expr_t parse_from_str(char* input) {
    debug("Parsing: %s\n", input);
    Parser_t parser = (Parser_t)allocate_mem("parse_from_str", NULL,
        sizeof(struct Parser));
    parser->stream = input;
    parser->counter = 0;
    parser->can_free_stream = FALSE;
    
    Expr_t result = parse(parser);
    free_parser(&parser);
    return result;
}

Expr_t parse(Parser_t parser) {
    debug_start("parse (counter: %llu)\n", parser->counter);

    struct Token token = get_next_token(parser);
    Expr_t result = NULL;
    switch (token.type) {
        case Symbol: {
            result = expr_make_atom(token.str);
            break;
        }
        case OpenParen: {
            Expr_t list = expr_make_empty_list();
            Expr_t child = parse(parser);
            while (child != NULL) {
                expr_add_to_list(list, child);
                child = parse(parser);
            }
            result = list;
            break;
        }
        default: {
            break;
        }
    }
    free_token(token);
    debug_end("/parse\n");
    return result;
}

