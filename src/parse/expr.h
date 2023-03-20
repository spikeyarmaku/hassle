#ifndef _EXPR_H_
#define _EXPR_H_

#include "config.h"
#include "global.h"

#include "memory.h"

#include "parse\dict.h"

/*
Expr usage:

Construction:
  To construct an Expr, an ExprBuilder must be used. One can be created by
  calling `make_expr_builder`, optionally giving it a dict (this allows a dict
  to be reused between multiple parsers). If no dict is given, an empty one will
  be created. Then, tokens can be added with `append_token`, and after the last
  one is added, `finalize_builder` must be called, so that the inner dict and
  expr won't take up more space than necessary.

Destruction:
  ExprBuilder does not need to be destructed after use, but the expr and dict
  components need to be freed up if they are not in use anymore. Destroying an
  expr is done via `free_expr`.
*/

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

struct _ExprBuilder {
    Expr expr;
    struct Dict dict;

    size_t _expr_size;   // Size of allocated memory for expr
    size_t _expr_cursor; // Address of the next empty expr slot
    size_t _dict_size;   // Size of allocated memory for dict
};

typedef struct _ExprBuilder* ExprBuilder;

// The reason a dictionary is passed to make_expr_builder is because it allows
// a dictionary to be re-used between multiple parses
ExprBuilder         make_expr_builder   (struct Dict*);

ErrorCode           finalize_builder    (ExprBuilder);
ErrorCode           find_symbol         (ExprBuilder, char*, size_t*);
ErrorCode           append_token        (ExprBuilder, uint8_t, char*);
ErrorCode           _grow_expr          (ExprBuilder);
ErrorCode           _finalize_expr      (ExprBuilder);

char*               lookup_symbol_by_id (Expr, struct Dict);
size_t              _bytes_to_index     (Expr);
    
void                free_expr_builder   (ExprBuilder);
void                free_expr           (Expr*);
BOOL                is_equal_expr       (Expr, Expr);
size_t              match_size          (Expr, Expr);
size_t              match_size_bytes    (Expr, Expr);
    
void                print_expr          (Expr, struct Dict, char*);
    
BOOL                is_list             (Expr);
BOOL                is_empty_list       (Expr);

Expr                advance_token       (Expr);
Expr                advance_expr        (Expr);

#endif