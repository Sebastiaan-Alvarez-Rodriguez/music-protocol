#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "communication/packet/packet.h"
#include "communication/checksums/checksum.h"
#include "com.h"

///////////////////////////////////////////////////
// Important - Read me
//Raw buffer convention:
// 0      15 - 16 31 - 32     63 - sizeof(data)-1
// checksum1 - size  - checksum2 - data
// Total buffer size is 64 + data length, and
///////////////////////////////////////////////////

// Get checksum1 from raw received buffer
static uint16_t buf_get_checksum1(const void* const buf) {
    return *(uint16_t*) buf;
}

// Get size from raw received buffer
static uint16_t buf_get_size(const void* const buf) {
    const uint16_t* pointer = buf;
    ++pointer;
    return *pointer;
}

// Get checksum2 from raw received buffer
static uint32_t buf_get_checksum2(const void* const buf) {
    const uint32_t* pointer = buf;
    ++pointer;
    return *pointer;
}

// Get data-pointer from raw received buffer
static const void* buf_get_data(const void* const buf) {
    const uint64_t* pointer = buf;
    ++pointer;
    return pointer;
}

static uint16_t make_checksum1(uint16_t sizefield, uint32_t checksum2field) {
    uint16_t buf[3];
    buf[0] = sizefield;
    buf[1] = checksum2field & 0xffff;
    buf[2] = checksum2field >> 16;
    return generate_16bit_fletcher(buf, sizeof(uint16_t)+sizeof(uint32_t));
}

static uint32_t make_checksum2(const void* const buffer, uint16_t buffersize) {
    return generate_32bit_fletcher(buffer, buffersize);
}

// Convert a packet_t to be sent with sendcom-function
// Returns true on success, false otherwise
// On success, sets pointer to created buffer, and to size of buffur
static bool convert_send(void** buf, uint16_t* const size, const packet_t* const packet) {
    *size = sizeof(uint16_t)*2 + sizeof(uint32_t) + packet->size;
    *buf = malloc(*size);
    if (*buf == NULL || errno == ENOMEM)
        return false;
    bzero(*buf, *size);

    // Compute checksums
    uint32_t checksum2 = make_checksum2(packet->data, packet->size);
    uint16_t checksum1 = make_checksum1(packet->size, checksum2);

    printf("Checksum2: %u\n", checksum2);
    printf("Checksum1: %u\n", checksum1);

    uint16_t* pointer = *buf;
    *pointer = checksum1;            // Write checksum1 field
    ++pointer;                       // Move to size field
    *pointer = packet->size;         // Write size field
    ++pointer;                       // Move to checksum2 field
    *pointer = checksum2;            // Write checksum2 field
    pointer+=2;                      // Move to data field
    memcpy(pointer, packet->data, packet->size);
    return true;
}

// Convert a buffer, received with recvcom-function, to a packet_t.
// Returns true on success (and sets pointer), false otherwise
// Here, size parameter should correspond to size of data section
static bool convert_recv(packet_t* const out, const void* const data, uint16_t size) {
    out->size = size;
    out->data = malloc(size);
    if (out->data == NULL || errno == ENOMEM)
        return false;

    const void* pointer = buf_get_data(data);
    memcpy(out->data, pointer, size);

    return true;
}

//Print all bits for given size in buffer. assumes little endian
static void print_bits(const size_t size, const void* const ptr) {
    uint8_t* b = (uint8_t*) ptr;

    for (int i=size-1;i>=0;i--)
        for (int j=7;j>=0;j--) {
            uint8_t byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    puts("");
}

void init_com(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address) {
    com->sockfd = sockfd;
    com->flags = flags;

    com->address = address;
    com->addr_len = sizeof(*address);
    com->packet = malloc(sizeof(packet_t));
}

bool send_com(const com_t* const com) {
    void* buf = NULL;
    uint16_t size = 0;
    if (!convert_send(&buf, &size, com->packet)) {
        perror("convert_send");
        return false;
    }

    printf("Sending %p\n", buf);
    printf("Size: %u\n", size);
    printf("Size data: %u\n", com->packet->size);
    printf("Size other: %u\n", size - com->packet->size);

    puts("Raw data:");
    print_bits(size, buf);

    bool ret = sendto(com->sockfd, buf, size, com->flags, com->address, com->addr_len) >= 0;
    free(buf);
    if(!ret)
        perror("sendto");
    return ret;
}

bool receive_com(com_t* const com) {
    void* check_buf = malloc(sizeof(uint16_t)*2+sizeof(uint32_t));
    if (check_buf == NULL || errno == ENOMEM)
        return false;

    //Peek at checksum1 and size and checksum2
    if(recvfrom(com->sockfd, check_buf, sizeof(uint16_t)*2+sizeof(uint32_t), MSG_PEEK, com->address, &com->addr_len) < 0)
        return false;
    uint16_t checksum1 = buf_get_checksum1(check_buf);
    uint16_t size = buf_get_size(check_buf);
    uint32_t checksum2 = buf_get_checksum2(check_buf);
    free(check_buf);

    // TODO: check checksum!

    //Get all received data
    void* full_data = malloc(sizeof(uint16_t)*2+sizeof(uint32_t)+size);
    if (full_data == NULL || errno == ENOMEM)
        return false;
    if(recvfrom(com->sockfd, full_data, sizeof(uint16_t)*2+sizeof(uint32_t)+size, com->flags, com->address, &com->addr_len) < 0)
        return false;

    printf("Received %p\n", full_data);
    printf("Size: %lu\n", sizeof(uint16_t)*2+sizeof(uint32_t)+size);
    printf("Size data: %u\n", size);
    printf("Size other: %lu\n", sizeof(uint16_t)*2+sizeof(uint32_t));
    puts("Raw data:");
    print_bits(sizeof(uint16_t)*2+sizeof(uint32_t)+size, full_data);

    convert_recv(com->packet, full_data, size);
    free(full_data);
    return true;
}

void free_com(const com_t* const com) {
    free(com->packet->data);
    free(com->packet);
}
