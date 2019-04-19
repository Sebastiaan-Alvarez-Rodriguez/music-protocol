#ifndef CHECKSUM
#define CHECKSUM

#include <stdint.h>

uint16_t generate(const void* const data, size_t size){
    uint8_t* pointer = data;

    uint16_t sum1 = 0xff, sum2 = 0xff;
    while (size) {
        size_t tlen = size > 20 ? 20 : size;
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

#endif