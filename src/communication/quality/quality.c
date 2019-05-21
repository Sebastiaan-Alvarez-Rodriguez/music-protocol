#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "communication/constants/constants.h"
#include "quality.h"

static void quality_reset_stats(quality_t* const quality) {
    quality->lost = 0;
    quality->faulty = 0;
    quality->ok = 0;
    quality->last_measure = 0;
}

// 1. Quality should only change if at least 64 packets have been sent
// 2. Quality should increase if less than 6.25% of packets had problems
// 3. Quality should decrease if more than 12.5% of packets had problems
void quality_init(quality_t* const quality, uint8_t inital_quality) {
    quality->current = inital_quality;
    quality_reset_stats(quality);
}

inline bool quality_should_increase(const quality_t* const quality) {
    return quality->ok > 16*(quality->lost + quality->faulty);
}

inline bool quality_should_decrease(const quality_t* const quality) {
    return quality->ok < 8*(quality->lost + quality->faulty);
}

inline bool quality_suggest_downsampling(const quality_t* const quality) {
    return quality->current == 1;
}

inline bool quality_suggest_compression(const quality_t* const quality) {
    return quality->current <= 2;
}

bool quality_adjust(quality_t* const quality) {
    quality->last_measure+=1;
    if (quality->last_measure != 5)
        return false;

    bool changed = false;
    if (quality->current < 5 && quality_should_increase(quality)) {
        quality->current += 1;
        quality_reset_stats(quality);
        changed = true;
    } else if (quality->current > 1 && quality_should_decrease(quality)) {
        quality->current -= 1;
        quality_reset_stats(quality);
        changed = true;
    } else {
        quality_reset_stats(quality);
    }
    return changed;
}