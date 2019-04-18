#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "communication/packets/udp.h"
#include "com.h"


//Raw buffer convention:
// 0 15 - 16    31 - 32+sizeof(data)-1
// size - checksum - data
// Total buffer size is always 16*2 + data length, and divisible by 16

// Convert a udp_t to be sent with sendcom-function
// Returns true on success, false otherwise
// Returns pointer to created buffer, and to size of buf on success
static bool convert_send(void* buf, size_t* const size, const udp_t* const udp_packet) {
    uint16_t* pointer = buf;
    *pointer = udp_packet->packet->size;   // Write size field
    pointer += sizeof(uint16_t);           // Move to checksum field
    *pointer = udp_packet->checksum;       // Write checksum field
    pointer += sizeof(uint16_t);           // Move to data field
    memcpy(pointer, udp_packet->packet->data, udp_packet->packet->size);
    return true;
}

// Convert a buffer, received with recvcom-function, to udp_t.
// Returns true on success (and sets pointer), false otherwise
static bool convert_recv(udp_t* const out, const void* const data, uint16_t size, uint16_t checksum) {
    out->packet->size = size;
    out->packet->data = malloc(size);
    if (out->packet->data == NULL || errno == ENOMEM)
        return false;

    out->checksum = checksum;
    memcpy(out->packet->data, data, size);     //Copy data section to packet

    return true;
}

// Get checksum from raw received buffer
static uint16_t buf_get_checksum(const void* const buf) {
    const uint16_t* pointer = buf;
    pointer+=sizeof(uint16_t);
    return *pointer;
}

void init_com(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address, socklen_t addr_len) {
    com->sockfd = sockfd;
    com->flags = flags;

    com->address = address;
    com->addr_len = addr_len;

    com->udp_packet = malloc(sizeof(udp_t));
    com->udp_packet->packet = malloc(sizeof(packet_t));
}

bool send_com(const com_t* const com) {
    if (com->udp_packet->packet->size % 16 != 0) {
        fprintf(stderr, "size = %u\n", com->udp_packet->packet->size);
        errno = EINVAL;
        return false;
    }

    size_t size = sizeof(uint16_t) * 2 + com->udp_packet->packet->size;
    void* buf = malloc(size);

    if (buf == NULL || errno == ENOMEM)
        return false;


    if (!convert_send(buf, &size, com->udp_packet))
        return false;

    bool ret = sendto(com->sockfd, buf, size, com->flags, com->address, com->addr_len) >= 0;
    free(buf);
    return ret;
}

bool receive_com(com_t* const com) {
    void* size_checksum_buf = malloc(sizeof(uint16_t)*2);
    if (size_checksum_buf == NULL || errno == ENOMEM)
        return false;

    //Get size and checksum
    recvfrom(com->sockfd, size_checksum_buf, sizeof(uint16_t)*2, com->flags, com->address, &com->addr_len);
    uint16_t size = *(uint16_t*) size_checksum_buf;
    uint16_t checksum = buf_get_checksum(size_checksum_buf);
    free(size_checksum_buf);

    //Get data
    void* data_buf = malloc(size);
    if(recvfrom(com->sockfd, data_buf, sizeof(size), com->flags, com->address, &com->addr_len) < 0) {
        perror("recvfrom");
        return false;
    }

    convert_recv(com->udp_packet, data_buf, size, checksum);
    free(data_buf);
    return true;
}

void free_com(const com_t* const com) {
    free(com->udp_packet->packet);
    free(com->udp_packet);
}
