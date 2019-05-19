#ifndef SIMULATION
#define SIMULATION

#include <stdint.h>
#include <stdbool.h>

void simulate_swap_packets(unsigned* const packet_nrs, const size_t batch_len, const float probability);

void simulate_flip_bits(void* const data, const uint16_t size, const float probability);

size_t simulate_random_wait_amt(const unsigned min, const unsigned max);

bool simulate_random_chance(const float probability);
#endif