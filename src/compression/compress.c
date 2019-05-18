#include <string.h>
#include <stdio.h>
#include "communication/com.h"
#include "compress.h"

// Reduces 128 bits to 112 bits, achieving compression of 12.5%
// without great loss of audio quality, as with downsampling
static void pack_128bit(const void* const data, void* const new_data) {
    const uint16_t* data_ptr = data;
    uint16_t* new_ptr = new_data;
    *new_ptr = (*data_ptr & 0xFFFC) | ((*(data_ptr+1) & 0xC000) >> 14);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x3FFC) << 2) | ((*(data_ptr+1) & 0xF000) >> 12);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x0FFC) << 4) | ((*(data_ptr+1) & 0xFC00) >> 10);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x03FC) << 6) | ((*(data_ptr+1) & 0xFF00) >> 8);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x00FC) << 8) | ((*(data_ptr+1) & 0xFFC0) >> 6);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x003C) << 10) | ((*(data_ptr+1) & 0xFFF0) >> 4);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*data_ptr & 0x000C) << 12) | ((*(data_ptr+1) & 0xFFFC) >> 2);
    // eight uint16_t is not needed
}

// Increases 112 bits packed to 128 bits, undoing packing
static void unpack_128bit(const void* const data, void* const new_data) {
    const uint16_t* data_ptr = data;
    uint16_t* new_ptr = new_data;
    *new_ptr = (*data_ptr & 0xFFFC);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x0003) << 14) | ((*data_ptr & 0xFFF0) >> 2);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x000F) << 12) | ((*data_ptr & 0xFFC0) >> 4);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x003F) << 10) | ((*data_ptr & 0xFF00) >> 6);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x00FF) << 8) | ((*data_ptr & 0xFC00) >> 8);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x03FF) << 6) | ((*data_ptr & 0xF000) >> 10);
    ++data_ptr;
    ++new_ptr;
    *new_ptr = ((*(data_ptr-1) & 0x0FFF) << 4) | ((*data_ptr & 0xC000) >> 12);
    ++new_ptr;
    *new_ptr = *data_ptr << 2;
}

void compress(com_t* const com) {
    uint16_t pairs_128_bits = com->packet->size / 16;//256/16=16
    void* new_data = malloc(pairs_128_bits*14);//16*14=224
    uint16_t* data_ptr = com->packet->data;
    uint16_t* new_ptr = new_data;
    for (unsigned i = 0; i < pairs_128_bits; ++i) {
        pack_128bit(data_ptr, new_ptr);
        data_ptr+=8; //move 128 bits
        new_ptr+=7; //move 112 bits (last 16 not needed because compression)
    }
    com->packet->data = new_data;
    com->packet->size = pairs_128_bits*14;
}

void decompress(com_t* const com) {
    uint16_t pairs_112_bits = com->packet->size / 14;//224/14=16
    void* new_data = malloc(pairs_112_bits*16);//16*16=256

    uint16_t* data_ptr = com->packet->data;
    uint16_t* new_ptr = new_data;
    for (unsigned i = 0; i < pairs_112_bits; ++i) {
        unpack_128bit(data_ptr, new_ptr);
        data_ptr+=7; //move 112 bits (last 16 not needed because compression)
        new_ptr+=8;  //move 128 bits
    }
    com->packet->data = new_data;
    com->packet->size = pairs_112_bits*16;
}

void downsample(com_t* const com, const size_t n) {
    const uint8_t frame_length = 16*2;
    const uint16_t newsize = (com->packet->size / n) * (n-1);
    void* compressed = malloc(newsize);
    uint8_t* compressed_ptr = compressed;
    uint8_t* data_ptr = com->packet->data;
    size_t count2 = 0;

    size_t frames = com->packet->size / frame_length;
    for (size_t i = 0; i < frames; ++i) {
        if (i % n == 0)
            continue;
        memcpy(compressed_ptr, data_ptr, frame_length);
        compressed_ptr += frame_length;
        data_ptr += frame_length;
        count2 += frame_length;
    }
    com->packet->size = newsize;
    com->packet->data = compressed;
}