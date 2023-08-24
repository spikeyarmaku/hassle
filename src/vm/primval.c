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
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_string", NULL,
        sizeof(struct PrimVal));
    result->type = StringValue;
    result->string = str;
    return result;
}

PrimVal_t* primval_make_reference(char* sym) {
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_reference", NULL,
        sizeof(struct PrimVal));
    result->type = ReferenceValue;
    result->string = sym;
    return result;
}

PrimVal_t* primval_make_symbol(char* sym) {
    PrimVal_t* result = (PrimVal_t*)allocate_mem("primval_make_reference", NULL,
        sizeof(struct PrimVal));
    result->type = SymbolValue;
    result->string = sym;
    return result;
}

enum PrimValType primval_get_type(PrimVal_t* val) {
    assert(val != NULL);
    return val->type;
}

Rational_t* primval_get_rational(PrimVal_t* val) {
    assert(val != NULL);
    assert(val->type == RationalValue);
    return val->rational;
}

char* primval_get_string(PrimVal_t* val) {
    assert(val != NULL);
    assert(val->type == StringValue);
    return val->string;
}

char* primval_get_symbol(PrimVal_t* val) {
    assert(val != NULL);
    assert(val->type == SymbolValue);
    return val->string;
}

char* primval_get_reference(PrimVal_t* val) {
    assert(val != NULL);
    assert(val->type == ReferenceValue);
    return val->string;
}

PrimVal_t* primval_copy(PrimVal_t* primval) {
    assert(primval != NULL);
    switch (primval->type) {
        case RationalValue:
            return primval_make_rational(rational_copy(primval->rational));
        case StringValue:
            return primval_make_string(str_cpy(primval->string));
        case ReferenceValue:
            return primval_make_reference(str_cpy(primval->string));
        case SymbolValue:
            return primval_make_symbol(str_cpy(primval->string));
        default: assert(FALSE); return NULL;
    }
}

void primval_serialize(Serializer_t* serializer, PrimVal_t* primval) {
    assert(primval != NULL);
    assert(serializer != NULL);
    serializer_write(serializer, (uint8_t)primval->type);
    switch (primval->type) {
        case RationalValue:
            rational_serialize(serializer, primval->rational);
            break;
        case StringValue:
        case ReferenceValue:
        case SymbolValue:
            serializer_write_string(serializer, primval->string);
            break;
        default:
            break;
    }
}

PrimVal_t* primval_deserialize(Serializer_t* serializer) {
    assert(serializer != NULL);
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
        case ReferenceValue: {
            char* ref = serializer_read_string(serializer);
            return primval_make_reference(ref);
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

void primval_print(PrimVal_t* primval) {
    assert(primval != NULL);
    switch(primval->type) {
        case RationalValue: {
            rational_print(primval->rational);
            break;
        }
        case StringValue: {
            printf("\"%s\"", primval->string);
            break;
        }
        case ReferenceValue: {
            printf("%s", primval->string);
            break;
        }
        case SymbolValue: {
            printf("%s", primval->string);
            break;
        }
    }
}

// void primval_free(PrimVal_t* val) {
//     assert(val != NULL);
//     switch (val->type) {
//         case RationalValue:
//             rational_free(val->rational);
//             break;
//         case StringValue:
//         case ReferenceValue:
//             free_mem("value_free/symbol", val->string);
//             break;
//         default:
//             assert(FALSE);
//     }

//     free_mem("primval_free", val);
// }
