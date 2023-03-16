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

struct _Token {
    enum TokenType type;
    char* str;
};

struct _Parser {
    char* stream;
    long int counter;
};

ErrorCode       parse_from_file     (char*, Expr*, struct Dict*);
ErrorCode       parse_from_str      (char*, Expr*, struct Dict*);

struct _Parser  _create_parser      (char*);
void            _free_parser        (struct _Parser*);

ErrorCode       _get_next_token     (struct _Parser*, struct _Token*);
ErrorCode       _read_symbol        (struct _Parser*, struct _Token*);
ErrorCode       _read_string        (struct _Parser*, struct _Token*);
ErrorCode       _read_identifier    (struct _Parser*, struct _Token*);
void            _free_token         (struct _Token*);

char            _get_next_char      (struct _Parser*);
char            _get_current_char   (struct _Parser*);

BOOL            _is_whitespace      (char);
void            _consume_whitespace (struct _Parser*);

ErrorCode       _parse              (struct _Parser, Expr*, struct Dict*);

#endif