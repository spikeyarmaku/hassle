#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdlib.h>
#include <stdio.h>

#include "expr.h"

enum TokenType {OpenParen, CloseParen, Symbol, Eos};

struct Token {
    enum TokenType type;
    char* str;
    int size;
};

struct Parser {
    char* stream;
    long int counter;
    long int size;
};

int get_next_token(struct Parser*, struct Token*);
int read_symbol(struct Parser*, struct Token*);
int read_string(struct Parser*, struct Token*);
int read_identifier(struct Parser*, struct Token*);
char get_next_char(struct Parser*);
char get_current_char(struct Parser*);
int is_whitespace(char);
int consume_whitespace(struct Parser*);

int parse(struct Parser, struct Expr*);
int parse_list(struct Parser, struct Expr*);

#endif