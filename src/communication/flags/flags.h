#ifndef FLAGS
#define FLAGS
#include <stdint.h>

enum flags {
    FLAG_NONE= 0x0,
    FLAG_ACK = 0x1,
    FLAG_REJ = 0x2,
    FLAG_RR  = 0x4
}; 
// Acknowledge
// Ready to Receive
// Reject all remaining frames

// Returns a uint16_t containing <amount> specified flags
// Returns 0 if FLAGS_NONE is in flags
uint16_t flags_get_raw(unsigned amount, ...);

bool flags_is_NONE(uint16_t rawflags);

bool flags_is_ACK(uint16_t rawflags);

bool flags_is_REJ(uint16_t rawflags);

bool flags_is_RR(uint16_t rawflags);
#endif