#ifndef COMPRESS
#define COMPRESS

#include <stdbool.h>
#include <stdlib.h>
#include "communication/com.h"

void downsample(com_t* const com, const size_t n);

void compress(com_t* const com);

void decompress(com_t* const com);

#endif