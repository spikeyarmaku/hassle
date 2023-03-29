#include "global.h"

#ifdef DEBUG_PRINTS
uint64_t debug_level = 0;
#endif

void error(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    #endif
}

void debug(int8_t level, const char* s, ...) {
    #ifdef DEBUG_PRINTS
    // printf("%llu ", debug_level);
    if (level > 0) debug_level += level;
    if (debug_level <= DEBUG_LEVEL) {
        va_list args;
        va_start(args, s);
        for (uint64_t i = 0; i < debug_level; i++) {
            printf(" ");
        }
        vprintf(s, args);
        va_end(args);
    }
    if (level < 0) debug_level += level;
    #endif
}

INDEX bytes_to_index(uint8_t* bytes, uint8_t count) {
    INDEX result = 0;
    INDEX multiplier = 1;
    for (uint8_t i = 0; i < count; i++) {
        result += bytes[i] * multiplier;
        multiplier <<= 8;
    }
    return result;
}

void index_to_bytes(INDEX index, uint8_t* buffer, uint8_t count) {
    INDEX temp = 255;
    for (uint8_t i = 0; i < count; i++) {
        buffer[i] = (index & temp) >> (8 * i);
        temp <<= 8;
    }
}
