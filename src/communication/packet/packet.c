#include "packet.h"
#include <stdlib.h>

void init_packet(packet_t* const packet, uint16_t flags) {
    packet->flags = flags;
    packet->size = 0;
    packet->data = NULL;
}