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
    uint8_t flags;
    struct sockaddr* address;
    socklen_t addr_len;
} com_t;

// Initialize a com-struct
void com_init(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address, uint8_t packet_flags, uint8_t packetnr);

// Send a packet from the server
// Includes SIMULATION define
bool com_send_server(const com_t* const com);

// Send a packet to destined client
// Returns true on success, false otherwise (should check errno)
bool com_send(const com_t* const com);

// Receive a packet from server
// Returns true on success, false otherwise (checksum fail or malloc fail)
bool com_receive(com_t* const com, const bool consume_if_fail);

// Wait until a packet from server is received (or timeout)
// Returns true upon successful receive, false otherwise
bool com_receive_peek(const com_t* const com);

// Free a given com_t structure
void com_free(const com_t* const com);

void com_print(const com_t* const com);

#endif
