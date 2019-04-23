#ifndef PACKET
#define PACKET
#include <stdint.h>

typedef struct {
    uint16_t size;
    uint8_t flags;
    uint8_t nr;
    void* data;
} packet_t;

// Initialize a packet_t-struct
void packet_init(packet_t* const packet, uint8_t flags, uint8_t packetnr);

#endif