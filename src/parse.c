#include "parse.h"

struct Parser create_parser(char* file_name) {
    struct Parser p;
    FILE* fp = fopen(file_name, "r");

    // Get file size
    rewind(fp);
    long int start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp) - start;
    rewind(fp); // fseek(fp, 0L, SEEK_SET);

    char* file_content = (char*)alloc_mem(sizeof(char) * (size + 1));
    
    size_t chars_read = fread(file_content, sizeof(char), size, fp);
    
    // Add terminating null.
    // Sometimes multiple bytes are read, but one byte is reported (like on
    // Windows "\r\n" is read as "\n"). Thus, `chars_read is used instead of
    // `size`.
    file_content[chars_read] = 0;

    fclose(fp);

    p.stream = file_content;
    p.counter = 0;
    return p;
}

void destroy_parser(struct Parser p) {
    free_mem(p.stream);
}

// Reads the next token, and sets the stream pointer to the next non-whitespace
// character.
uint8_t get_next_token(struct Parser* p, struct Token* t) {
    t->str = NULL;
    int error = 0;
    
    // Decide what type of token to read based on the first character
    char c = get_current_char(p);
    if (c == 0) {
        t->type = Eos;
    } else if (c == '(') {
        t->type = OpenParen;
    } else if (c == ')') {
        t->type = CloseParen;
    } else {
        error = read_symbol(p, t);
    }
    
    // If it is a simple token, advance the stream pointer
    if (t->type != Symbol) {
        get_next_char(p);
    }

    // Set the stream pointer to the next non-whitespace character
    consume_whitespace(p);

    return error;
}

// Read a string (enclosed in quotes) or an identifier
uint8_t read_symbol(struct Parser* p, struct Token* t) {
    int error = 0;

    char c = get_current_char(p);
    if (c == '"') {
        // String
        error = read_string(p, t);
    } else {
        // Not a string, disallowed characters: "() and whitespace
        error = read_identifier(p, t);
    }
    return error;
}

// Read a string enclosed in quotes and trailed by at least one whitespace
// character or a closing parenthesis or EOF
uint8_t read_string(struct Parser* p, struct Token* t) {
    long int starting_position = p->counter;
    long int ending_position = p->counter;
    int escape_counter = 0;
    char c = get_next_char(p);

    // Find the end position of the string
    int is_escaped = 0;
    int go_on = 1;
    while (go_on) {
        if (is_escaped == 0) {
            if (c == '"') {
                go_on = 0;
                c = get_next_char(p);
                ending_position = p->counter;
            } else {
                if (c == '\\') {
                    // escape a character
                    is_escaped = 1;
                    escape_counter++;
                }
                c = get_next_char(p);
            }
        } else {
            c = get_next_char(p);
            is_escaped = 0;
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = get_current_char(p);
    if ((!is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        return 1;
    }

    // Copy the string into a buffer, without the escape characters
    long int total_char_count = ending_position - starting_position;
    long int char_count = total_char_count - escape_counter;
    escape_counter = 0;
    is_escaped = 0;
    char* str = (char*)alloc_mem(sizeof(char) * (char_count + 1));
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
    return 0;
}

// Read an identifier that is either trailed by a whitespace character or a
// closing parenthesis or EOF
uint8_t read_identifier(struct Parser* p, struct Token* t) {
    long int starting_position = p->counter;
    long int ending_position = p->counter;
    char c = get_current_char(p);

    // Find the end position of the identifier
    int go_on = 1;
    while (go_on) {
        if (c == '\\' || c == '(' || c == ')' || c == '"' || c == ';'
                || is_whitespace(c) || c == 0) {
            // escape a character
            go_on = 0;
            ending_position = p->counter;
        } else {
            c = get_next_char(p);
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis or EOF, and the size of the identifier
    // is greater than zero
    c = get_current_char(p);
    if ((!is_whitespace(c) && c != ')' && c != 0) ||
            ending_position - starting_position == 0) {
        return 1;
    }

    // Copy the string into a buffer
    long int char_count = ending_position - starting_position;
    char* str = (char*)alloc_mem(sizeof(char) * (char_count + 1));
    for (long int i = 0; i < char_count; i++) {
        char c = p->stream[starting_position + i];
        str[i] = c;
    }
    str[char_count] = 0; // Terminating null

    // Fill the token
    t->type = Symbol;
    t->str = str;
    return 0;
}

char get_next_char(struct Parser* p) {
    if (p->stream[p->counter] != 0) {
        p->counter++;
    }
    return get_current_char(p);
}

char get_current_char(struct Parser* p) {
    if (p->counter >= 0) {
        return p->stream[p->counter];
    } else {
        return 0;
    }
}

uint8_t is_whitespace(char c) {
    return c == ' '  || c == '\n' || c == '\t'
        || c == '\v' || c == '\f' || c == '\r';
}

// Set the pointer to the next non-whitespace character
void consume_whitespace(struct Parser* p) {
    int go_on = 1;
    char c;
    while (go_on) {
        // Read the next character
        c = get_current_char(p);
        // If it is not a whitespace, or it is EOF, stop
        if (!is_whitespace(c) || c == 0) {
            return;
        } else {
            get_next_char(p);
        }
    }
}

struct Expr* parse_from_file(char* file_name) {
    struct Parser parser = create_parser(file_name);

    return parse(parser);
}

struct Expr* parse_from_str(char* input) {
    struct Parser parser;
    parser.stream = input;
    parser.counter = 0;
    
    return parse(parser);
}

struct Expr* parse(struct Parser parser) {
    struct Expr* expr = NULL;
    struct Stack* stack = NULL;
    struct Token t;

    int go_on = 1;
    while (go_on) {
        if (get_next_token(&parser, &t)) {
            error("parse: error reading next token\n");
            break;
        }

        switch(t.type) {
            case OpenParen: {
                expr = append(expr);
                expr->type = ExprList;
                push(&stack, expr);
                expr = NULL;
                break;
            }
            case CloseParen: {
                struct Expr* old_expr = pop(&stack);
                struct Expr* head = get_head(expr);
                old_expr->list = head;
                expr = old_expr;
                break;
            }
            case Symbol: {
                expr = append(expr);
                expr->type = ExprAtom;
                expr->symbol = t.str;
                break;
            }
            case Eos: {
                go_on = 0;
                break;
            }
        }
    }
    destroy_parser(parser);
    return get_head(expr);
}

void print_expr(struct Expr* expr) {
    if (expr->type == ExprList) {
        printf("(");
        expr = expr->list;
        while (expr != NULL) {
            print_expr(expr);
            if (expr->next != NULL) {
                putchar(' ');
            }
            expr = expr->next;
        }
        printf(")");
    } else {
        printf("%s", expr->symbol);
    }
}
