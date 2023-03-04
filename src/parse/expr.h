#ifndef _EXPR_H_
#define _EXPR_H_

#include <string.h>
#include <stdint.h>

#include "config.h"
#include "global.h"

#include "memory.h"

// S-Expressions are stored in two different places: the `Expr` datatype
// contains the actual structure of the s-expression, and the `Dict` datatype
// contains the names of the symbols. The reason for this distinction is
// memory efficiency, as it allows it to be used on embedded devices, by
// throwing away a lot of data (such as symbol names) that is not strictly
// necessary for execution - at the cost of e.g. harder debuggability.

// A bitpacked / bytepacked version of s-expr representation.
// If BITPACK_EXPR is defined: Two bits define the type of the next expression
// (00 - opening paren, 01 - closing paren, 10 - symbol, 11 - EOS).
// If it is 01, the next N bits define an ID for a symbol.
// If BITPACK_EXPR is not defined: 1 byte defines the type for the next
// expression, and 2 bytes define an ID for a symbol.
// Based on https://people.csail.mit.edu/rivest/Sexp.txt
typedef uint8_t* Expr;

enum TokenType {Eos, OpenParen, CloseParen, Symbol};

// A list of symbols, appearing in the list in the order of their ID, so the
// first element's ID is 0, the next one's ID is 1, etc.
struct Dict {
    size_t count;
    char** names;
};

struct ExprBuilder {
    Expr expr;
    struct Dict dict;

    size_t expr_size;   // Size of allocated memory for expr
    size_t expr_cursor; // Address of the next empty expr slot
    size_t dict_size;   // Size of allocated memory for dict
};

struct Dict         make_dict           ();
// The reason a dictionary is passed to make_expr_builder is because it allows
// a dictionary to be re-used between multiple parses
struct ExprBuilder  make_expr_builder   (struct Dict*);

ErrorCode grow_expr             (struct ExprBuilder*);
ErrorCode finalize_expr         (struct ExprBuilder*);
ErrorCode grow_dict             (struct ExprBuilder*);
ErrorCode finalize_dict         (struct ExprBuilder*);
ErrorCode append_symbol         (struct ExprBuilder*, char*, size_t*);
ErrorCode find_symbol           (struct ExprBuilder*, char*, size_t*);
ErrorCode append_token          (struct ExprBuilder*, uint8_t, char*);

size_t  bytes_to_index      (Expr, size_t);
char*   lookup_symbol_id    (Expr, size_t, struct Dict);
void    destroy_dict        (struct Dict);

void    destroy_expr    (Expr);
uint8_t is_equal_expr   (Expr, Expr);
size_t  match_size      (Expr, Expr);

void    print_expr      (Expr, struct Dict, char*);

#endif