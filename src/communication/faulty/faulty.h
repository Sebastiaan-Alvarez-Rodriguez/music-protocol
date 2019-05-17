#ifndef FAULTY
#define FAULTY

#include <stddef.h>

void randomize_packet_order(unsigned* const packet_nrs, const size_t batch_len, const float probability);

void flip_random_bits(const size_t size_data, void* const data, const size_t num_bits, const float probability);

#endif
