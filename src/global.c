#include "global.h"

#ifdef DEBUG_PRINTS
uint64_t debug_level = 0;
BOOL is_debug = TRUE;
#endif

void error(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    #endif
}

void debug_print(const char* s, va_list args) {
    #ifdef DEBUG_PRINTS
    // printf("%llu ", debug_level);
    if (is_debug) {
        for (uint64_t i = 0; i < debug_level; i++) {
            printf("    ");
        }
        vprintf(s, args);
    }
    #endif
}

void debug(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    va_list args;
    va_start(args, s);
    debug_print(s, args);
    va_end(args);
    #endif
}

void debug_start(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    va_list args;
    va_start(args, s);
    debug_print(s, args);
    va_end(args);

    debug_level++;
    #endif
}

void debug_end(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    debug_level--;
    
    va_list args;
    va_start(args, s);
    debug_print(s, args);
    va_end(args);
    #endif
}

void debug_on() {
    #ifdef DEBUG_PRINTS
    is_debug = TRUE;
    #endif
}

void debug_off() {
    #ifdef DEBUG_PRINTS
    is_debug = FALSE;
    #endif
}

WORD bytes_to_word(uint8_t* bytes, uint8_t count) {
    WORD result = 0;
    WORD multiplier = 1;
    for (uint8_t i = 0; i < count; i++) {
        result += bytes[i] * multiplier;
        multiplier <<= 8;
    }
    return result;
}

void word_to_bytes(WORD word, uint8_t* buffer, uint8_t count) {
    WORD temp = 255;
    for (uint8_t i = 0; i < count; i++) {
        buffer[i] = (word & temp) >> (8 * i);
        temp <<= 8;
    }
}

char* str_cpy(char* src) {
    char* result = (char*)allocate_mem("str_cpy", NULL,
        sizeof(char) * (strlen(src) + 1));
    strcpy(result, src);
    return result;
}
