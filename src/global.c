#include "global.h"

inline int error(const char* s, ...)
{
    #ifdef DEBUG
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    #endif
}
