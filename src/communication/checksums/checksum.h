#ifndef CHECKSUM
#define CHECKSUM

#include <stdint.h>

uint16_t generate_16bit_fletcher(const void* const data, uint16_t size);

uint32_t generate_32bit_fletcher(const void* const data, uint16_t size);

#endif