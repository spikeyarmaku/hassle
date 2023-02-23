#include "global.h"

int error(const char* s, ...) {
    #ifdef DEBUG
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    #else
    return 0;
    #endif
}
