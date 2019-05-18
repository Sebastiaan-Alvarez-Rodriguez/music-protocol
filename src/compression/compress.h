#ifndef COMPRESS
#define COMPRESS

#include <stdbool.h>
#include <stdlib.h>
#include "communication/com.h"

// Give a com you want to send, and a n > 1
// Throw away 1 frame every n frames
// Remember, make sure n > 1
void downsample(com_t* const com, const size_t n);

// Compress data inside a com
// Compression is done by throwing away the least significant 2 bits
// from every 16 bits, filling up with most significant 2 from next 16 bits
void compress(com_t* const com);

//Undoes compression, such that everything is half-word alligned once more
void decompress(com_t* const com);

#endif