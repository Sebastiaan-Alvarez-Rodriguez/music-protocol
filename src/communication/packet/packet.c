#include "packet.h"
#include <stdlib.h>

void packet_init(packet_t* const packet, uint8_t flags, uint8_t packetnr) {
    packet->size = 0;
    packet->flags = flags;
    packet->nr = packetnr;
    packet->data = NULL;
}

void packet_reset(packet_t* const packet) {
    packet->size = 0;
    packet->flags = 0;
    packet->nr = 0;
}
