#ifndef QUALITY
#define QUALITY

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t lost;
    size_t faulty;
    size_t ok;
    uint8_t current;
    size_t last_measure;
} quality_t;

// Initialize a quality struct
void quality_init(quality_t* const quality, uint8_t inital_quality);

// Returns true if quality should increase, otherwise false
bool quality_should_increase(const quality_t* const quality);

// Returns true if quality should decrease, otherwise false
bool quality_should_decrease(const quality_t* const quality);

// Returns true if quality suggests downsampling must be used, otherwise false
bool quality_suggest_downsampling(const quality_t* const quality);

// Returns true if quality suggests compression must be used, otherwise false
bool quality_suggest_compression(const quality_t* const quality);

// Main function to call after each batch collection. 
// Handles quality change computations and history.
// Returns true if quality changed, otherwise false
bool quality_adjust(quality_t* const quality);
#endif