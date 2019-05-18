#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "communication/constants/constants.h"
#include "quality.h"

// 1. Quality should only change if at least 64 packets have been sent
// 2. Quality should increase if more than 67% of packets arrived ok
// 3. Quality should decrease if less than 50% of packets arrived ok
void quality_init(quality_t* const quality, uint8_t inital_quality) {
    quality->lost = 0;
    quality->faulty = 0;
    quality->ok = 0;
    quality->current = inital_quality;
}

inline bool quality_should_increase(const quality_t* const quality) {
    size_t total = quality->ok + quality->lost + quality->faulty;
    return total >= 64 && (quality->ok / 2)*3 > total;
}

inline bool quality_should_decrease(const quality_t* const quality) {
    size_t total = quality->ok + quality->lost + quality->faulty;
    return total >= 64 && quality->ok*2 < total;
}

inline bool quality_suggest_downsampling(const quality_t* const quality) {
    return quality->current <= 1;
}

inline bool quality_suggest_compression(const quality_t* const quality) {
    return quality->current <= 2;
}

bool quality_adjust(quality_t* const quality) {
    puts("Quality adjustment time");
    printf("Current quality is %u\n", quality->current);
    printf("Lost: %lu\nFaulty: %lu\nOK: %lu\n", quality->lost, quality->faulty, quality->ok);
    if (quality->current < 5 && quality_should_increase(quality)) {
        puts("Quality should increase");
        quality->current += 1;
        quality->lost = 0;
        quality->faulty = 0;
        quality->ok = 0;
        return true;
    } else if (quality->current > 1 && quality_should_decrease(quality)) {
        puts("Quality should decrease");
        quality->current -= 1;
        quality->lost = 0;
        quality->faulty = 0;
        quality->ok = 0;
        return true;
    }
    return false;
}