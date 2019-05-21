#ifndef SIMULATION
#define SIMULATION

#include <stdint.h>
#include <stdbool.h>

void simulate_swap_packets(unsigned* const packet_nrs, const size_t batch_len, const float probability);

// Takes a raw buffer, size of this buffer
// and probability for a flip to occur in a bit
// After calling, data might be changed
void simulate_flip_bits(void* const data, const uint16_t size, const float probability);

// Returns an (unsigned) integer between min and max
// to be used as sleep amount
size_t simulate_random_wait_amt(const unsigned min, const unsigned max);

// Function which generates a random number
// Returns true if number is less than given probability. Otherwise false
bool simulate_random_chance(const float probability);
#endif