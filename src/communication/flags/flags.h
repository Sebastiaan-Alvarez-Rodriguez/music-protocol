#ifndef FLAGS
#define FLAGS
#include <stdint.h>

enum flags {
    FLAG_NONE= 0x0,
    FLAG_ACK = 0x1,
    FLAG_REJ = 0x2,
    FLAG_RR  = 0x4,
    FLAG_EOS = 0x8
};
// Acknowledge
// Ready to Receive
// Reject all remaining frames

// Returns a uint8_t containing <amount> specified flags
// Returns 0 if FLAGS_NONE is in flags
uint8_t flags_get_raw(unsigned amount, ...);

bool flags_is_NONE(uint8_t rawflags);

bool flags_is_ACK(uint8_t rawflags);

bool flags_is_REJ(uint8_t rawflags);

bool flags_is_RR(uint8_t rawflags);

bool flags_is_EOS(uint8_t rawflags);
#endif
