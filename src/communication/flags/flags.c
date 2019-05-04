#include <stdarg.h>
#include <stdbool.h>
#include "flags.h"

inline uint8_t flags_get_raw(unsigned amount, ...) {
    va_list args;
    va_start(args, amount);
    uint8_t returnvalue = 0;
    for (unsigned i = 0; i < amount; ++i)
        returnvalue = returnvalue | va_arg(args, enum flags);
    va_end(args);
    return returnvalue;
}

inline bool flags_is_NONE(uint8_t rawflags) {
    return rawflags == 0;
}

inline bool flags_is_ACK(uint8_t rawflags) {
    return (rawflags & 0x1) == 1;
}

inline bool flags_is_REJ(uint8_t rawflags) {
    return ((rawflags & 0x2) >> 1) == 1;
}

inline bool flags_is_RR(uint8_t rawflags) {
    return ((rawflags & 0x4) >> 2) == 1;
}

inline bool flags_is_EOS(uint8_t rawflags) {
    return ((rawflags & 0x8) >> 3) == 1;
}
