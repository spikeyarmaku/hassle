#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <stdint.h>

#include "global.h"
#include "parse\expr.h"

/*
Parser usage:

Either call `parse_from_file` or `parse_from_str`, which returns an Expr_t.
*/
enum TokenType {Eos, OpenParen, CloseParen, Symbol};

typedef struct Parser* Parser_t;

Expr_t  parse_from_file (char*);
Expr_t  parse_from_str  (char*);

#endif