#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "communication/packet/packet.h"
#include "communication/checksums/checksum.h"
#include "communication/simulation/simulation.h"
#include "com.h"

///////////////////////////////////////////////////
// Important - Read me
// Raw buffer convention:
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
    return *(((uint16_t*)buf)+1);
}

// Get flags from raw buffer
static inline uint8_t buf_get_flags(const void* const buf) {
    return *(((uint8_t*)buf)+4);
}

// Get flags from raw buffer
static inline uint8_t buf_get_packetnr(const void* const buf) {
    return *(((uint8_t*)buf)+5);
}

// Get checksum2 from raw buffer
static inline uint16_t buf_get_checksum2(const void* const buf) {
    const uint16_t* pointer = buf;
    pointer += 3;
    return *(((uint16_t*)buf)+3);
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
static void* convert_send(uint16_t* const size, const packet_t* const packet) {
    *size = sizeof(uint16_t)*4 + packet->size;
    void* buf = malloc(*size);
    if (buf == NULL || errno == ENOMEM)
        return NULL;

    // Compute checksums
    uint16_t checksum2 = make_checksum2(packet->data, packet->size);
    uint16_t checksum1 = make_checksum1(packet->size, packet->flags, packet->nr, checksum2);

    uint16_t* pointer = buf;
    *pointer = checksum1;            // Write checksum1 field
    ++pointer;                       // Move to size field
    *pointer = packet->size;         // Write size field
    ++pointer;                       // Move to flags field
    *pointer = ((packet->nr << 8) | packet->flags);  // Write flags & packet_nr field
    ++pointer;                       // Move to checksum2 field
    *pointer = checksum2;            // Write checksum2 field
    ++pointer;                       // Move to data field
    memcpy(pointer, packet->data, packet->size);
    return buf;
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
__attribute__ ((unused)) static void print_hex(const size_t size, const void* const ptr) {
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

bool com_send(const com_t* const com) {
    uint16_t size = 0;

    void* buf = convert_send(&size, com->packet);
    if (!buf) {
        perror("convert_send");
        return false;
    }

    #ifdef SIMULATE
    if (com->packet->flags != 0x1) {
        #ifdef SIMULATE_DROP_PACKET_CHANCE
        if (simulate_random_chance(SIMULATE_DROP_PACKET_CHANCE)) {
            #ifdef SIMULATE_PRINT
            if (com->packet->flags != 0)
                printf("DROPPING A PACKET with flags: %x\n", com->packet->flags);
            else
                puts("DROPPING A PACKET");
            #endif
            free(buf);
            return true;
        }
        #endif
        #ifdef SIMULATE_BIT_SOME_FLIP_CHANCE
        if (simulate_random_chance(SIMULATE_BIT_SOME_FLIP_CHANCE)) {
            #ifdef SIMULATE_PRINT
            printf("FLIP RANDOM BITS\n");
            #endif
            simulate_flip_bits(buf, size, SIMULATE_BIT_FLIP_CHANCE);
        }
        #endif
        #ifdef SIMULATE_RANDOM_WAIT_CHANCE
        if (simulate_random_chance(SIMULATE_RANDOM_WAIT_CHANCE)) {
            size_t amt = simulate_random_wait_amt(SIMULATE_RANDOM_WAIT_MIN, SIMULATE_RANDOM_WAIT_MAX);
            #ifdef SIMULATE_PRINT
            printf("SLEEPING for %lu ms\n", amt);
            #endif
            usleep(amt);
        }
        #endif
    }
    #endif

    bool ret = sendto(com->sockfd, buf, size, com->flags, com->address, com->addr_len) >= 0;
    free(buf);
    if(!ret)
        perror("sendto");
    return ret;
}

enum recv_flag com_receive(com_t* const com) {
    void* check_buf = malloc(sizeof(uint16_t)*4);
    if (check_buf == NULL || errno == ENOMEM)
        return RECV_ERROR;

    //Peek at checksum1 and size and checksum2
    if(recvfrom(com->sockfd, check_buf, sizeof(uint16_t)*4, MSG_PEEK, com->address, &com->addr_len) < 0) {
        free(check_buf);
        if (errno == EAGAIN) { //we had a timeout
            errno = 0;
            return RECV_TIMEOUT;
        }
        return RECV_ERROR;
    }

    uint16_t checksum1 = buf_get_checksum1(check_buf);
    uint16_t size = buf_get_size(check_buf);
    uint8_t flags = buf_get_flags(check_buf);
    uint8_t packetnr = buf_get_packetnr(check_buf);
    uint16_t checksum2 = buf_get_checksum2(check_buf);
    free(check_buf);
    //Checksum control for checksum 1
    uint16_t test_checksum1 = make_checksum1(size, flags, packetnr, checksum2);
    if (checksum1 != test_checksum1) {
        recvfrom(com->sockfd, check_buf, 0, MSG_WAITALL, com->address, &com->addr_len);
        return RECV_FAULTY;
    }

    //Get all received data
    void* full_data = malloc(sizeof(uint16_t)*4+size);
    if (full_data == NULL || errno == ENOMEM)
        return RECV_ERROR;
    if(recvfrom(com->sockfd, full_data, sizeof(uint16_t)*4+size, com->flags, com->address, &com->addr_len) < 0) {
        free(full_data);
        return RECV_ERROR;
    }

    //Checksum control for checksum2
    uint16_t test_checksum2 = make_checksum2(buf_get_data(full_data), size);
    if (checksum2 != test_checksum2) {
        free(full_data);
        return RECV_FAULTY;
    }

    convert_recv(com->packet, full_data, size, flags, packetnr);
    free(full_data);
    return RECV_OK;
}

bool com_receive_peek(const com_t* const com) {
    return recvfrom(com->sockfd, NULL, 0, MSG_PEEK, NULL, 0) < 0;
}

void com_consume_packet(const com_t* const com) {
    recvfrom(com->sockfd, NULL, sizeof(uint16_t)*4+com->packet->size, com->flags, NULL, NULL);
}

void com_free(const com_t* const com) {
    free(com->packet);
}