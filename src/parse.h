#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "expr.h"
#include "stack.h"

enum TokenType {OpenParen, CloseParen, Symbol, Eos};

struct Token {
    enum TokenType type;
    char* str;
};

struct Parser {
    char* stream;
    long int counter;
};

struct Parser   create_parser       (char*);
void            destroy_parser      (struct Parser);

uint8_t         get_next_token      (struct Parser*, struct Token*);
uint8_t         read_symbol         (struct Parser*, struct Token*);
uint8_t         read_string         (struct Parser*, struct Token*);
uint8_t         read_identifier     (struct Parser*, struct Token*);

char            get_next_char       (struct Parser*);
char            get_current_char    (struct Parser*);

uint8_t         is_whitespace       (char);
void            consume_whitespace  (struct Parser*);

struct Expr*    parse_from_file     (char*);
struct Expr*    parse_from_str      (char*);
struct Expr*    parse               (struct Parser);

void            print_expr          (struct Expr*);

#endif