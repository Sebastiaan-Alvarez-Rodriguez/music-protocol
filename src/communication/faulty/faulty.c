#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "faulty.h"
#include "stdbool.h"

static void swap(unsigned* array, unsigned a, unsigned b) {
    unsigned c = array[a];
    array[a] = array[b];
    array[b] = c;
}

void randomize_packet_order(unsigned* const packet_nrs, const size_t batch_len, const float probability) {
    for(unsigned i = 0; i < batch_len; ++i) {
        float probability_swapped = (rand() % 101) / 100.0f;
        if(probability_swapped < probability)
            swap(packet_nrs, packet_nrs[i], packet_nrs[rand() % batch_len]);
    }
    // printf("[");
    // for(unsigned i = 0; i < batch_len; ++i) {
    //     printf("%u,", packet_nrs[i]);
    // }
    // puts("]");
}

// Get checksum1 from raw buffer
static uint16_t* buf_get_checksum1(void* buf) {
    return (uint16_t*) buf;
}

// Get size from raw buffer
static uint16_t* buf_get_size(void* buf) {
    uint16_t* pointer = buf;
    ++pointer;
    return pointer;
}

// Get flags from raw buffer
static uint8_t* buf_get_flags(void* buf) {
    uint32_t* pointer = buf;
    ++pointer;
    return (uint8_t*) pointer;
}

// Get flags from raw buffer
static uint8_t* buf_get_packetnr(void* buf) {
    uint8_t* pointer = buf;
    pointer += 5;
    return pointer;
}

// Get checksum2 from raw buffer
static uint16_t* buf_get_checksum2(void* buf) {
    uint16_t* pointer = buf;
    pointer += 3;
    return pointer;
}

// Get data-pointer from raw buffer
static uint8_t* buf_get_data(void* buf, const size_t byte_pos) {
    uint8_t* pointer = buf;
    pointer += 8 + byte_pos;
    return pointer;
}

void flip_random_bits(const size_t size_data, void* const data, const size_t num_bits, const float probability) {
    for(unsigned i = 0; i < num_bits; ++i) {
        uint8_t section = rand() % 6;
        // printf("section: %u\n", section);
        float bit_prob = (float)((rand() % 101) / 100.0f);
        // printf("%f < %f = %s\n", bit_prob, probability, bit_prob < probability ? "TRUE" : "FALSE");
        if(bit_prob < probability) {

            switch (section) {
                case 0:
                    (*buf_get_checksum1(data)) ^= (1UL << (rand() % 16));
                    break;
                case 1:
                    (*buf_get_size(data)) ^= (1UL << (rand() % 16));
                    break;
                case 2:
                    (*buf_get_flags(data)) ^= (1UL << (rand() % 8));
                    break;
                case 3:
                    (*buf_get_packetnr(data)) ^= (1UL << (rand() % 8));
                    break;
                case 4:
                    (*buf_get_checksum2(data)) ^= (1UL << (rand() % 16));
                    break;
                case 5:
                    (*buf_get_data(data, rand() % size_data)) ^= (1UL << (rand() % 8));
                    break;
            }
        }
    }
}
