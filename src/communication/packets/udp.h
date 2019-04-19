#ifndef UDP
#define UDP
#include <stdint.h>
#include "packet.h"

typedef struct {
    uint16_t checksum;
    packet_t* packet;
} udp_t;

#endif