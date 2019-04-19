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
// On success, sets pointer to created buffer, and one to size of buf
static bool convert_send(void** buf, size_t* const size, const udp_t* const udp_packet) {
    *size = sizeof(uint16_t)*2 + udp_packet->packet->size;
    *buf = malloc(*size);
    if (buf == NULL || errno == ENOMEM)
        return false;

    uint16_t* pointer = *buf;
    *pointer = udp_packet->packet->size;   // Write size field
    ++pointer;                             // Move to checksum field
    *pointer = udp_packet->checksum;       // Write checksum field
    ++pointer;                             // Move to data field
    memcpy(pointer, udp_packet->packet->data, udp_packet->packet->size);
    return true;
}

// Convert a buffer, received with recvcom-function, to udp_t.
// Returns true on success (and sets pointer), false otherwise
// Here, size parameter should correspond to size of data section
static bool convert_recv(udp_t* const out, const void* const data, uint16_t size, uint16_t checksum) {
    out->checksum = checksum;

    out->packet->size = size;
    out->packet->data = malloc(size);
    if (out->packet->data == NULL || errno == ENOMEM)
        return false;

    const uint16_t* datapointer = data;
    datapointer+= 2;                              //Move to data section of buffer
    memcpy(out->packet->data, datapointer, size); //Copy data section to packet

    return true;
}

// Get checksum from raw received buffer
static uint16_t buf_get_checksum(const void* const buf) {
    const uint16_t* pointer = buf;
    ++pointer;                      //Move pointer to checksum section
    return *pointer;
}

//Print all bits for given size in buffer. assumes little endian
static void printBits(const size_t size, const void* const ptr) {
    unsigned char* b = (unsigned char*) ptr;

    for (int i=size-1;i>=0;i--)
        for (int j=7;j>=0;j--) {
            unsigned char byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    puts("");
}

void init_com(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address) {
    com->sockfd = sockfd;
    com->flags = flags;

    com->address = address;
    com->addr_len = sizeof(*address);

    com->udp_packet = malloc(sizeof(udp_t));
    com->udp_packet->packet = malloc(sizeof(packet_t));
}

bool send_com(const com_t* const com) {
    if (com->udp_packet->packet->size % 16 != 0) {
        fprintf(stderr, "size = %u (mod 16 != 0)\n", com->udp_packet->packet->size);
        errno = EINVAL;
        return false;
    }

    void* buf = NULL;
    size_t size;
    if (!convert_send(&buf, &size, com->udp_packet)) {
        perror("convert_send");
        return false;
    }

    printf("Sending %p\n", buf);
    printf("Size: %lu\n", size);
    printf("Size data: %u\n", com->udp_packet->packet->size);
    printf("Size other: %u\n", 4);

    bool ret = sendto(com->sockfd, buf, size, com->flags, com->address, com->addr_len) >= 0;
    puts("Raw data:");
    printBits(size, buf);
    free(buf);
    if(!ret)
        perror("sendto");
    return ret;
}

bool receive_com(com_t* const com) {
    void* size_checksum_buf = malloc(sizeof(uint16_t)*2);
    if (size_checksum_buf == NULL || errno == ENOMEM)
        return false;

    //Peek at size and checksum
    if(recvfrom(com->sockfd, size_checksum_buf, sizeof(uint16_t)*2, MSG_PEEK, com->address, &com->addr_len) < 0)
        return false;
    uint16_t size = *(uint16_t*) size_checksum_buf;
    uint16_t checksum = buf_get_checksum(size_checksum_buf);
    free(size_checksum_buf);

    // TODO: check checksum!

    //Get all received data
    void* full_data = malloc(sizeof(uint16_t)*2+size);
    if(recvfrom(com->sockfd, full_data, sizeof(uint16_t)*2+size, com->flags, com->address, &com->addr_len) < 0)
        return false;

    printf("Received %p\n", full_data);
    printf("Size: %lu\n", sizeof(uint16_t)*2+size);
    printf("Size data: %u\n", size);
    printf("Size other: %u\n", 4);
    puts("Raw data:");
    printBits(sizeof(uint16_t)*2+size, full_data);
    //convert to a com
    convert_recv(com->udp_packet, full_data, size, checksum);
    free(full_data);
    return true;
}

void free_com(const com_t* const com) {
    free(com->udp_packet->packet);
    free(com->udp_packet);
}
