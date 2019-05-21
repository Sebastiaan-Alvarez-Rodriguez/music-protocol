#ifndef SIMULATION
#define SIMULATION

#include <stdint.h>
#include <stdbool.h>

#define SIMULATE
#ifdef SIMULATE
    // Use only if you want to print verbose information about simulation
    // #define SIMULATE_PRINT

    // Use if you want to simulate packet reordering
    #define SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE 35.0f
    #ifdef SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE
        #define SIMULATE_RANDOMIZE_PACKET_ORDER_SWAP_CHANCE 25.0f
    #endif

    // Use if you want to simulate bit flipping
    // #define SIMULATE_BIT_SOME_FLIP_CHANCE 8.0f
    #ifdef SIMULATE_BIT_SOME_FLIP_CHANCE
        #define SIMULATE_BIT_FLIP_CHANCE 15.0f
    #endif

    // Use if you want to simulate packet dropping
    #define SIMULATE_DROP_PACKET_CHANCE 5.0f

    // Use if you want to simulate packet waiting
    #define SIMULATE_RANDOM_WAIT_CHANCE 5.0f
    #ifdef SIMULATE_RANDOM_WAIT_CHANCE
        // Times below represent ms, must be integer
        #define SIMULATE_RANDOM_WAIT_MIN 1
        #define SIMULATE_RANDOM_WAIT_MAX 10
    #endif
#endif

/*
// #define SIMULATE
#ifdef SIMULATE
    // Use only if you want to print verbose information about simulation
    // #define SIMULATE_PRINT

    // Use if you want to simulate packet reordering
    #define SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE 35.0f
    #ifdef SIMULATE_RANDOMIZE_PACKET_ORDER_CHANCE
        #define SIMULATE_RANDOMIZE_PACKET_ORDER_SWAP_CHANCE 25.0f
    #endif

    // Use if you want to simulate bit flipping
    // #define SIMULATE_BIT_SOME_FLIP_CHANCE 8.0f
    #ifdef SIMULATE_BIT_SOME_FLIP_CHANCE
        #define SIMULATE_BIT_FLIP_CHANCE 15.0f
    #endif

    // Use if you want to simulate packet dropping
    #define SIMULATE_DROP_PACKET_CHANCE 5.0f

    // Use if you want to simulate packet waiting
    #define SIMULATE_RANDOM_WAIT_CHANCE 5.0f
    #ifdef SIMULATE_RANDOM_WAIT_CHANCE
        // Times below represent ms, must be integer
        #define SIMULATE_RANDOM_WAIT_MIN 1
        #define SIMULATE_RANDOM_WAIT_MAX 10
    #endif
#endif
*/

void simulate_randomize_packet_order(unsigned* const packet_nrs, const size_t batch_len, const float probability);

// Takes a raw buffer, size of this buffer
// and probability for a flip to occur in a bit
// After calling, data might be changed
void simulate_flip_bits(void* const data, const uint16_t size, const float probability);

// Returns an (unsigned) integer between min and max
// to be used as sleep amount
size_t simulate_random_wait_amt(const unsigned min, const unsigned max);

// Function which generates a random number
// Returns true if number is less than given probability. Otherwise false
bool simulate_random_chance(const float probability);
#endif