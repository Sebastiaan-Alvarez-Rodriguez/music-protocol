#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "communication/com.h"

#include "simulation.h"

com_t* captured;

static void swap(unsigned* array, unsigned a, unsigned b) {
    unsigned c = array[a];
    array[a] = array[b];
    array[b] = c;
}


void simulate_swap_packets(unsigned* const packet_nrs, const size_t batch_len, const float probability) {
    for(unsigned i = 0; i < batch_len; ++i) {
        float probability_swapped = (rand() % 101) / 100.0f;
        if(probability_swapped < probability)
            swap(packet_nrs, packet_nrs[i], packet_nrs[rand() % batch_len]);
    }
}

void simulate_flip_bits(void* const data, const uint16_t size, const float probability) {
    for(unsigned i = 0; i < size * 8; ++i) {
        float bit_prob = (float)((rand() % 100)+1);
        if(bit_prob < probability) {
            puts("FLIPPING A BIT");
            unsigned location_byte = rand() % size;
            unsigned location_bit = rand() % 8;
            uint8_t* data_ptr = ((uint8_t*) data) + location_byte;
            *data_ptr ^= (uint8_t)(1UL << location_bit); 
        }
    }
}

inline bool simulate_drop_packet(const float probability) {
    bool ans = ((float)((rand() % 100)+1)) < probability;
    if (ans)
        puts("DROPPING A PACKET");
    return ans;
}