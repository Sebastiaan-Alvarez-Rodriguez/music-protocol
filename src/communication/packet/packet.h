#ifndef PACKET
#define PACKET
#include <stdint.h>

typedef struct {
    uint16_t size;
    uint16_t flags;
    void* data;
} packet_t;

// Initialize a packet_t-struct
void init_packet(packet_t* const packet, uint16_t flags);

#endif