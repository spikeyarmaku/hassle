#include "primval.h"

struct PrimVal {
    enum PrimValType type;
    union {
        Rational_t* rational;
        char* string;
    };
};

PrimVal_t* primval_make_rational(Rational_t* rat) {
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_rational", NULL,
        sizeof(struct PrimVal));
    result->type = RationalValue;
    result->rational = rat;
    return result;
}

PrimVal_t* primval_make_string(char* str) {
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_rational", NULL,
        sizeof(struct PrimVal));
    result->type = StringValue;
    result->string = str;
    return result;
}

PrimVal_t* primval_make_symbol(char* sym) {
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_rational", NULL,
        sizeof(struct PrimVal));
    result->type = SymbolValue;
    result->string = sym;
    return result;
}

enum PrimValType primval_get_type(PrimVal_t* val) {
    return val->type;
}

Rational_t* primval_get_rational(PrimVal_t* val) {
    assert(val->type == RationalValue);
    return val->rational;
}

char* primval_get_string(PrimVal_t* val) {
    assert(val->type == StringValue);
    return val->string;
}

char* primval_get_symbol(PrimVal_t* val) {
    assert(val->type == SymbolValue);
    return val->string;
}

PrimVal_t* primval_copy(PrimVal_t* primval) {
    switch (primval->type) {
        case RationalValue:
            return primval_make_rational(rational_copy(primval->rational));
        case StringValue:
        case SymbolValue:
            return primval_make_string(str_cpy(primval->string));
        default: assert(FALSE); return NULL;
    }
}

void primval_serialize(Serializer_t* serializer, PrimVal_t* primval) {
    serializer_write(serializer, (uint8_t)primval->type);
    switch (primval->type) {
        case RationalValue:
            printf("Serializing rational primval\n");
            rational_serialize(serializer, primval->rational);
            break;
        case StringValue:
        case SymbolValue:
            printf("Serializing string /symbol primval\n");
            serializer_write_string(serializer, primval->string);
            break;
        default:
            break;
    }
}

PrimVal_t* primval_deserialize(Serializer_t* serializer) {
    enum PrimValType type = (enum PrimValType)serializer_read(serializer);
    switch (type) {
        case RationalValue: {
            Rational_t* rational = rational_deserialize(serializer);
            return primval_make_rational(rational);
        }
        case StringValue: {
            char* string = serializer_read_string(serializer);
            return primval_make_string(string);
        }
        case SymbolValue: {
            char* symbol = serializer_read_string(serializer);
            return primval_make_symbol(symbol);
        }
        default: {
            assert(FALSE);
            return NULL;
        }
    }
}

void primval_free(PrimVal_t* val) {
    assert(val != NULL);
    switch (val->type) {
        case RationalValue:
            rational_free(val->rational);
            break;
        case StringValue:
        case SymbolValue:
            free_mem("value_free/symbol", val->string);
            break;
        default:
            assert(FALSE);
    }

    free_mem("primval_free", val);
}
