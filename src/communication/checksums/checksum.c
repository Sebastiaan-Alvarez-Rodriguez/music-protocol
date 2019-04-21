#include <stdint.h>
#include "checksum.h"

uint16_t generate_16bit_fletcher(const void* const data, uint16_t size) {
    const uint8_t* pointer = data;

    uint16_t sum1 = 0xff, sum2 = 0xff;
    while (size) {
        uint16_t tlen = size > 20 ? 20 : size;
        size -= tlen;
        do {
            sum2 += sum1 += *pointer++;
        } while (--tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }

    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return sum2 << 8 | sum1;
}

uint32_t generate_32bit_fletcher(const void* const data, uint16_t size) {
    uint16_t words = size/2;
    
    const uint16_t* pointer = data;

    uint32_t sum1 = 0xffff, sum2 = 0xffff;
    while (words) {
            uint16_t tlen = words > 359 ? 359 : words;
            words -= tlen;
            do {
                sum2 += sum1 += *pointer++;
            } while (--tlen);
            sum1 = (sum1 & 0xffff) + (sum1 >> 16);
            sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }
    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}