#include "global.h"

void error(const char* s, ...) {
    #ifdef DEBUG_PRINTS
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    #endif
}

void debug(int level, const char* s, ...) {
    #ifdef DEBUG_PRINTS
    if (level <= DEBUG_LEVEL) {
        va_list args;
        va_start(args, s);
        vprintf(s, args);
        va_end(args);
    }
    #endif
}
