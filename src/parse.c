#include "parse.h"

// Reads the next token, and sets the stream pointer to the next non-whitespace
// character.
int get_next_token(struct Parser* p, struct Token* t) {
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
    long int space_counter = consume_whitespace(p);

    return error;
}

// Read a string enclosed in quotes or an identifier
int read_symbol(struct Parser* p, struct Token* t) {
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
// character or a closing parenthesis
int read_string(struct Parser* p, struct Token* t) {
    int starting_position = p->counter;
    int ending_position = p->counter;
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
    // character or a closing parenthesis, and the size of the identifier is
    // greater than zero
    c = get_current_char(p);
    if ((!is_whitespace(c) && c != ')') ||
            ending_position - starting_position == 0) {
        return 1;
    }

    // Copy the string into a buffer, without the escape characters
    int total_char_count = ending_position - starting_position;
    int char_count = total_char_count - escape_counter;
    escape_counter = 0;
    is_escaped = 0;
    char* str = (char*)malloc(sizeof(char) * char_count);
    for (int i = 0; i < total_char_count; i++) {
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

    // Fill the token
    t->str = str;
    t->type = Symbol;
    t->size = char_count;
    return 0;
}

// Read an identifier that is either trailed by a whitespace character or a
// closing parenthesis
int read_identifier(struct Parser* p, struct Token* t) {
    int starting_position = p->counter;
    int ending_position = p->counter;
    char c = get_current_char(p);

    // Find the end position of the identifier
    int go_on = 1;
    while (go_on) {
        if (c == '\\' || c == '(' || c == ')' || c == '"' || c == ';'
                || is_whitespace(c)) {
            // escape a character
            go_on = 0;
            ending_position = p->counter;
        } else {
            c = get_next_char(p);
        }
    }

    // Check if the character after the identifier is either a whitespace
    // character or a closing parenthesis, and the size of the identifier is
    // greater than zero
    c = get_current_char(p);
    if ((!is_whitespace(c) && c != ')') ||
            ending_position - starting_position == 0) {
        return 1;
    }

    // Copy the string into a buffer
    int char_count = ending_position - starting_position;
    char* str = (char*)malloc(sizeof(char) * char_count);
    for (int i = 0; i < char_count; i++) {
        char c = p->stream[starting_position + i];
        str[i] = c;
    }

    // Fill the token
    t->str = str;
    t->type = Symbol;
    t->size = char_count;
    return 0;
}

char get_next_char(struct Parser* p) {
    if (p->counter < p->size) {
        p->counter++;
    }
    return get_current_char(p);
}

char get_current_char(struct Parser* p) {
    if (p->counter < p->size) {
        return p->stream[p->counter];
    } else {
        return 0;
    }
}

int is_whitespace(char c) {
    return c == ' '  || c == '\n' || c == '\t'
        || c == '\v' || c == '\f' || c == '\r';
}

// Set the pointer to the next non-whitespace character, and report back the
// number of characters skipped.
int consume_whitespace(struct Parser* p) {
    long int counter = 0;
    int go_on = 1;
    char c;
    while (go_on) {
        // Read the next character
        c = get_current_char(p);
        // If it is not a whitespace, or it is EOF, stop
        if (!is_whitespace(c) || c == 0) {
            return counter;
        } else {
            get_next_char(p);
            counter++;
        }
    }
}

int parse(struct Parser parser, struct Expr* expr) {
    int go_on = 1;
    int error = 0;
    struct Token t;
    while (go_on) {
        if (get_next_token(&parser, &t)) {
            printf("Error\n");
            return 1;
        }
        switch (t.type) {
            case OpenParen:
                break;
            case CloseParen:
                break;
            case Symbol:
                break;
            case Eos:
                break;
        }
    }
    return 0;
}
