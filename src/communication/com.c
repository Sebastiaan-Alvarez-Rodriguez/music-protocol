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
// 0      15 - 16 31 - 32 39 - 40    47 - 48     63 - sizeof(data)-1
// checksum1 - size  - flags - packetnr - checksum2 - data
// Total buffer size is 64 + data length
///////////////////////////////////////////////////

// Get checksum1 from raw buffer
static inline uint16_t buf_get_checksum1(const void* const buf) {
    return *(uint16_t*) buf;
}

// Get size from raw buffer
static inline uint16_t buf_get_size(const void* const buf) {
    const uint16_t* pointer = buf;
    ++pointer;
    return *pointer;
}

// Get flags from raw buffer
static inline uint8_t buf_get_flags(const void* const buf) {
    const uint32_t* pointer = buf;
    ++pointer;
    return *(uint8_t*) pointer;
}

// Get flags from raw buffer
static inline uint8_t buf_get_packetnr(const void* const buf) {
    const uint8_t* pointer = buf;
    pointer += 5;
    return *pointer;
}

// Get checksum2 from raw buffer
static inline uint16_t buf_get_checksum2(const void* const buf) {
    const uint16_t* pointer = buf;
    pointer += 3;
    return *pointer;
}

// Get data-pointer from raw buffer
static inline const void* buf_get_data(const void* const buf) {
    const uint64_t* pointer = buf;
    ++pointer;
    return pointer;
}

static inline uint16_t make_checksum1(uint16_t sizefield, uint8_t flags, uint8_t packetnr, uint16_t checksum2field) {
    uint16_t buf[3];
    buf[0] = sizefield;
    buf[1] = (packetnr << 8) | flags;
    buf[2] = checksum2field;
    return generate_16bit_fletcher(buf, sizeof(buf));
}

static inline uint32_t make_checksum2(const void* const buffer, uint16_t buffersize) {
    return generate_16bit_fletcher(buffer, buffersize);
}

// Convert a packet_t to be sent with sendcom-function
// Returns true on success, false otherwise
// On success, sets pointer to created buffer, and to size of buffur
static bool convert_send(void** buf, uint16_t* const size, const packet_t* const packet) {
    *size = sizeof(uint16_t)*4 + packet->size;
    *buf = malloc(*size);
    if (*buf == NULL || errno == ENOMEM)
        return false;

    // Compute checksums
    uint16_t checksum2 = make_checksum2(packet->data, packet->size);
    uint16_t checksum1 = make_checksum1(packet->size, packet->flags, packet->nr, checksum2);

    uint16_t* pointer = *buf;
    *pointer = checksum1;            // Write checksum1 field
    ++pointer;                       // Move to size field
    *pointer = packet->size;         // Write size field
    ++pointer;                       // Move to flags field
    *pointer = ((packet->nr << 8) | packet->flags);  // Write flags & packet_nr field
    ++pointer;                       // Move to checksum2 field
    *pointer = checksum2;            // Write checksum2 field
    ++pointer;                       // Move to data field
    memcpy(pointer, packet->data, packet->size);
    return true;
}

// Convert a buffer, received with recvcom-function, to a packet_t.
// Returns true on success (and sets pointer), false otherwise
// Here, size parameter should correspond to size of data section
static bool convert_recv(packet_t* const out, const void* const data, uint16_t size, uint8_t flags, uint8_t packetnr) {
    out->size = size;
    out->flags = flags;
    out->nr = packetnr;
    out->data = malloc(size);
    if (out->data == NULL || errno == ENOMEM)
        return false;

    const void* pointer = buf_get_data(data);
    memcpy(out->data, pointer, size);

    return true;
}

//Print all bits for given size in buffer. assumes little endian
__attribute__ ((unused)) static void print_bits(const size_t size, const void* const ptr) {
    uint8_t* b = (uint8_t*) ptr;

    for (int i=size-1;i>=0;i--)
        for (int j=7;j>=0;j--) {
            uint8_t byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    puts("");
}

//Print all bits for given size in buffer. assumes little endian
static void print_hex(const size_t size, const void* const ptr) {
    uint8_t* hexptr = (uint8_t*) ptr;

    for (int i=size-1;i>=0;i--) {
            uint8_t byte = hexptr[i];
            printf("%X", byte);
        }
    puts("");
}

void com_init(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address, uint8_t packet_flags, uint8_t packetnr) {
    com->sockfd = sockfd;
    com->packet = malloc(sizeof(packet_t));
    com->flags = flags;
    com->address = address;
    com->addr_len = sizeof(struct sockaddr);
    packet_init(com->packet, packet_flags, packetnr);
}

// void com_init_addr_cpy(com_t* const com, unsigned sockfd, int flags, struct sockaddr* const address, uint8_t packet_flags, uint8_t packetnr) {
//     com->sockfd = sockfd;
//     com->packet = malloc(sizeof(packet_t));
//     com->flags = flags;
//     memcpy(com->address, address, );
//     com->addr_len = sizeof(struct sockaddr);
//     packet_init(com->packet, packet_flags, packetnr);
// }

bool com_send(const com_t* const com) {
    void* buf = NULL;
    uint16_t size = 0;
    if (!convert_send(&buf, &size, com->packet)) {
        perror("convert_send");
        return false;
    }

    // puts("-----SEND-----");
    // printf("Size: %u\n", size);
    // printf("Size data: %u\n", com->packet->size);
    // printf("Size other: %u\n", size - com->packet->size);
    // printf("flags: %#2x\n", com->packet->flags);
    // printf("packet nr: %u\n", com->packet->nr);
    // printf("Raw data:"); print_hex(size, buf);
    // puts("--------------");

    // if (com->flags == 0)
    //     print_hex(com->packet->size, com->packet->data);

    bool ret = sendto(com->sockfd, buf, size, com->flags, com->address, com->addr_len) >= 0;
    free(buf);
    if(!ret)
        perror("sendto");
    return ret;
}

bool com_receive(com_t* const com) {
    void* check_buf = malloc(sizeof(uint16_t)*4);
    if (check_buf == NULL || errno == ENOMEM)
        return false;

    //Peek at checksum1 and size and checksum2
    if(recvfrom(com->sockfd, check_buf, sizeof(uint16_t)*4, MSG_PEEK, com->address, &com->addr_len) < 0)
        return false;
    uint16_t checksum1 = buf_get_checksum1(check_buf);
    uint16_t size = buf_get_size(check_buf);
    uint8_t flags = buf_get_flags(check_buf);
    uint8_t packetnr = buf_get_packetnr(check_buf);
    uint16_t checksum2 = buf_get_checksum2(check_buf);

    //Checksum control for checksum 1
    uint16_t test_checksum1 = make_checksum1(size, flags, packetnr, checksum2);
    if (checksum1 != test_checksum1) {
        free(check_buf);
        printf("Checksum1 mismatch! Expected %#8X, got %#8X\n", checksum1, test_checksum1);
        return false;
    }
    free(check_buf);

    //Get all received data
    void* full_data = malloc(sizeof(uint16_t)*4+size);
    if (full_data == NULL || errno == ENOMEM)
        return false;
    if(recvfrom(com->sockfd, full_data, sizeof(uint16_t)*4+size, com->flags, com->address, &com->addr_len) < 0)
        return false;

    //Checksum control for checksum2
    uint16_t test_checksum2 = make_checksum2(buf_get_data(full_data), size);
    if (checksum2 != test_checksum2) {
        printf("Checksum2 mismatch! Expected %#8X, got %#8X", checksum2, test_checksum2);
        return false;
    }

    convert_recv(com->packet, full_data, size, flags, packetnr);
    free(full_data);

    // puts("-----RECV-----");
    // printf("Size: %lu\n", com->packet->size + sizeof(uint16_t)*4);
    // printf("Size data: %u\n", com->packet->size);
    // printf("Size other: %lu\n", sizeof(uint16_t)*4);
    // printf("flags: %#2x\n", com->packet->flags);
    // printf("packet nr: %u\n", packetnr);
    // printf("Raw data:"); print_hex(com->packet->size + sizeof(uint16_t)*4, com->packet);
    // puts("--------------");

    return true;
}

bool com_receive_peek(const com_t* const com) {
    // TODO: Add timeout!
    // TODO: Check: Gaat alles goed als ik het zo doe?
    return recvfrom(com->sockfd, NULL, 0, MSG_PEEK, NULL, 0) < 0;
}

void com_free(const com_t* const com) {
    free(com->packet);
}

void com_print(const com_t* const com) {
    struct sockaddr_in* ptr = (struct sockaddr_in*) com->address;
    printf("Com: {ip : %s}, {port : %u}, {family : %u}\n",
            inet_ntoa(ptr->sin_addr), ntohs(ptr->sin_port), ptr->sin_family);
}
