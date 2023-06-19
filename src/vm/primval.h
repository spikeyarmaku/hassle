#ifndef _PRIMVAL_H_
#define _PRIMVAL_H_

#include "rational\rational.h"

enum PrimValType {RationalValue, StringValue, SymbolValue};

typedef struct PrimVal PrimVal_t;

PrimVal_t*          primval_make_rational   (Rational_t*);
PrimVal_t*          primval_make_string     (char*);
PrimVal_t*          primval_make_symbol     (char*);
enum PrimValType    primval_get_type        (PrimVal_t*);
Rational_t*         primval_get_rational    (PrimVal_t*);
char*               primval_get_string      (PrimVal_t*);
char*               primval_get_symbol      (PrimVal_t*);
PrimVal_t*          primval_copy            (PrimVal_t*);
void                primval_print           (PrimVal_t*);
void                primval_free            (PrimVal_t*);

#endif