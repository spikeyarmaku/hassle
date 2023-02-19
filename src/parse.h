#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdlib.h>
#include <stdio.h>

#include "expr.h"
#include "stack.h"

enum TokenType {OpenParen, CloseParen, Symbol, Eos};

struct Token {
    enum TokenType type;
    char* str;
    long int size;
};

struct Parser {
    char* stream;
    long int counter;
    long int size;
};

struct Parser   create_parser       (char*);
void            destroy_parser      (struct Parser);

int             get_next_token      (struct Parser*, struct Token*);
int             read_symbol         (struct Parser*, struct Token*);
int             read_string         (struct Parser*, struct Token*);
int             read_identifier     (struct Parser*, struct Token*);

char            get_next_char       (struct Parser*);
char            get_current_char    (struct Parser*);

int             is_whitespace       (char);
int             consume_whitespace  (struct Parser*);

struct Expr*    parse_from_file     (char*);
struct Expr*    parse_from_str      (char*);
struct Expr*    parse               (struct Parser);

void            print_expr          (struct Expr*);

#endif