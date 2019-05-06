#ifndef COM
#define COM

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "communication/packet/packet.h"

typedef struct {
    unsigned sockfd;
    packet_t* packet;
    int flags;
    struct sockaddr* address;
    socklen_t addr_len;
} com_t;

// Initialize a com-struct
void com_init(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address, uint8_t packet_flags, uint8_t packetnr);

// Send a packet to destined client
// Returns true on success, false otherwise (should check errno)
bool send_com(const com_t* const com);

// Receive a packet from server
// Returns true on success, false otherwise (checksum fail or malloc fail)
bool receive_com(com_t* const com);

// Wait until a packet from server is received (or timeout)
// Returns true upon successful receive, false otherwise
bool receive_peek_com(const com_t* const com);

// Free a given com_t structure
void free_com(const com_t* const com);
#endif
