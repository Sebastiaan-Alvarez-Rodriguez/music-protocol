#ifndef QUALITY
#define QUALITY

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t lost;
    size_t faulty;
    size_t ok;
    uint8_t current;
} quality_t;

void quality_init(quality_t* const quality, uint8_t inital_quality);

bool quality_should_increase(const quality_t* const quality);

bool quality_should_decrease(const quality_t* const quality);

bool quality_suggest_downsampling(const quality_t* const quality);

bool quality_suggest_compression(const quality_t* const quality);

bool quality_adjust(quality_t* const quality);
#endif