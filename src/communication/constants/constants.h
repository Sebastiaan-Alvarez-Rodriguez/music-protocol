#ifndef CONSTANTS
#define CONSTANTS

#include <unistd.h>

// Returns size of one batch, in bytes.
// Returnvalue depends on given quality
size_t constants_batch_size(unsigned quality);

// Returns amount of packets per batch
// Returnvalue depends on given quality
size_t constants_batch_packets_amount(unsigned quality);

// Returns constant data size in bytes. This is the amount of bytes
// to be send to the client
size_t constants_packets_size();

#endif