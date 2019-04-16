#ifndef COM
#define COM

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "communication/packets/udp.h"

typedef struct {
    unsigned sockfd;
    udp_t* udp_packet;
    int flags;
} com_t;

// Send a udp_packet to destined client
// Returns true on success, false otherwise (should check errno)
bool sendcom(const com_t* const com);

// Receive a udp_packet from server
// Returns true on success, false otherwise (checksum fail or malloc fail)
bool receivecom(com_t* const com);

#endif