#ifndef CHECKSUM
#define CHECKSUM

#include <stdint.h>

// Generates a 16 bit fletcher checksum over a pointer, for given size
// Returns generated checksum
uint16_t generate_16bit_fletcher(const void* const data, uint16_t size);

// Generates a 32 bit fletcher checksum over a pointer, for given size
// Returns generated checksum
uint32_t generate_32bit_fletcher(const void* const data, uint16_t size);

#endif