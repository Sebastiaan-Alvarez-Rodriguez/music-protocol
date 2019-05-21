#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "communication/com.h"

#include "simulation.h"

static void swap(unsigned* array, unsigned a, unsigned b) {
    unsigned c = array[a];
    array[a] = array[b];
    array[b] = c;
}


void simulate_swap_packets(unsigned* const packet_nrs, const size_t batch_len, const float probability) {
    for(unsigned i = 0; i < batch_len; ++i) {
        float probability_swapped = (rand() % (100 + 1));
        if(probability_swapped < probability)
            swap(packet_nrs, packet_nrs[i], packet_nrs[rand() % batch_len]);
    }
}

void simulate_flip_bits(void* const data, const uint16_t size, const float probability) {
    for(unsigned i = 16; i < size; ++i) {
        float bit_prob = (float)(rand() % (100 + 1));
        if(bit_prob < probability) {
            unsigned location_bit = rand() % 8;
            uint8_t* data_ptr = ((uint8_t*) data) + i;
            *data_ptr ^= (uint8_t)(1UL << location_bit);
        }
    }
}

inline bool simulate_random_chance(const float probability) {
    return (float)((rand() % (100 + 1))) < probability;
}

inline size_t simulate_random_wait_amt(const unsigned min, const unsigned max) {
    return rand()%(max-min + 1) + min;
}
