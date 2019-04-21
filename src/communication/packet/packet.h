#ifndef PACKET
#define PACKET
#include <stdint.h>

typedef struct {
    uint16_t size;
    void* data;
} packet_t;

#endif