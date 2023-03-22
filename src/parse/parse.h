#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <stdint.h>

#include "global.h"
#include "parse\expr.h"

/*
Parser usage:

Either call `parse_from_file` or `parse_from_str`, which will output the
resulting expr and dict into the given addresses.
*/

enum TokenType {Eos, OpenParen, CloseParen, Symbol};

struct Token {
    enum TokenType type;
    char* str;
};

struct Parser {
    char* stream;
    long int counter;
};
typedef struct Parser* Parser_t;

Expr_t          parse_from_file     (ErrorCode_t*, char*);
Expr_t          parse_from_str      (ErrorCode_t*, char*);

Parser_t        _create_parser      (char*);
void            _free_parser        (Parser_t*);

struct Token    _get_next_token     (ErrorCode_t*, Parser_t);
ErrorCode_t     _read_symbol        (struct Token*, Parser_t);
ErrorCode_t     _read_string        (struct Token*, Parser_t);
ErrorCode_t     _read_identifier    (struct Token*, Parser_t);
void            _free_token         (struct Token*);

char            _get_next_char      (Parser_t);
char            _get_current_char   (Parser_t);

BOOL            _is_whitespace      (char);
void            _consume_whitespace (Parser_t);

Expr_t          _parse              (ErrorCode_t*, Parser_t);

#endif