#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <stdint.h>

#include "global.h"
#include "parse\expr.h"

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

ErrorCode       get_next_token      (struct Parser*, struct Token*);
ErrorCode       read_symbol         (struct Parser*, struct Token*);
ErrorCode       read_string         (struct Parser*, struct Token*);
ErrorCode       read_identifier     (struct Parser*, struct Token*);
void            destroy_token       (struct Token);

char            get_next_char       (struct Parser*);
char            get_current_char    (struct Parser*);

BOOL            is_whitespace       (char);
void            consume_whitespace  (struct Parser*);

ErrorCode       parse_from_file     (char*, Expr*, struct Dict*);
ErrorCode       parse_from_str      (char*, Expr*, struct Dict*);
ErrorCode       parse               (struct Parser, Expr*, struct Dict*);

#endif