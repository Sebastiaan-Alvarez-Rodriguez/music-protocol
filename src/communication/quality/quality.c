#include <stdint.h>
#include <stdbool.h>

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