#include <string.h>
#include <stdio.h>
#include <unistd.h>
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

//Print all bits for given size in buffer. assumes little endian
__attribute__ ((unused)) static void print_hex(const size_t size, const void* const ptr) {
    uint8_t* hexptr = (uint8_t*) ptr;

    for (int i=size-1;i>=0;i--) {
            uint8_t byte = hexptr[i];
            printf("%X", byte);
        }
    puts("");
}

//224/16=014
//014*14=196
void compress(com_t* const com, bool free_buf) {
    uint16_t pairs_128_bits = com->packet->size / 16;//256/16=16
    uint16_t newsize = pairs_128_bits*14;
    void* new_data = malloc(newsize);//16*14=224
    bzero(new_data, newsize);
    uint16_t* data_ptr = com->packet->data;
    uint16_t* new_ptr = new_data;
    for (unsigned i = 0; i < pairs_128_bits; ++i) {
        pack_128bit(data_ptr, new_ptr);
        data_ptr+=8; //move 128 bits
        new_ptr+=7; //move 112 bits (last 16 not needed because compression)
    }
    if (free_buf)
        free(com->packet->data);
    com->packet->data = new_data;
    com->packet->size = newsize;

}

//196/14=14
//14*16=224
void decompress(com_t* const com, bool free_buf) {
    uint16_t pairs_112_bits = com->packet->size / 14;
    uint16_t newsize = pairs_112_bits*16;
    void* new_data = malloc(newsize);
    bzero(new_data, newsize);
    uint16_t* data_ptr = com->packet->data;
    uint16_t* new_ptr = new_data;
    for (unsigned i = 0; i < pairs_112_bits; ++i) {
        unpack_128bit(data_ptr, new_ptr);
        data_ptr+=7; //move 112 bits (last 16 not needed because compression)
        new_ptr+=8;  //move 128 bits
    }
    if (free_buf)
        free(com->packet->data);
    com->packet->data = new_data;
    com->packet->size = newsize;
    if (newsize > 256)
        printf("DECOMPRESS CAUSES >255: %u\n", newsize);
}

void downsample(com_t* const com, const size_t n, bool free_buf) {
    const uint8_t frame_length = 4;
    const uint16_t newsize = (com->packet->size / n) * (n-1);
    void* compressed = malloc(newsize);
    bzero(compressed, newsize);
    uint32_t* compressed_ptr = compressed;
    uint32_t* data_ptr = com->packet->data;

    size_t frames = com->packet->size / frame_length;
    for (size_t i = 0; i < frames; ++i) {
        if (i % n == 0) {
            ++data_ptr;
            continue;
        }
        memcpy(compressed_ptr, data_ptr, sizeof(uint32_t));
        ++compressed_ptr;
        ++data_ptr;
    }
    if (free_buf)
        free(com->packet->data);
    com->packet->size = newsize;
    com->packet->data = compressed;
}

void resample(com_t* const com, const size_t n, bool free_buf) {
    const uint8_t frame_length = 4;
    const uint16_t newsize = (com->packet->size / (n-1)) * n;
    void* decompressed = malloc(newsize);
    bzero(decompressed, newsize);
    uint32_t* decompressed_ptr = decompressed;
    uint32_t* data_ptr = com->packet->data;
    // B C D E F G H J K L M N O P
    // 0 B C D E F G H 0 J K L M N O P
    size_t frames = newsize / frame_length;
    for (size_t i = 0; i < frames; ++i) {
        if (i % n == 0) {
            decompressed_ptr+=1;
            continue;
        }
        memcpy(decompressed_ptr, data_ptr, sizeof(uint32_t));
        ++decompressed_ptr;
        ++data_ptr;
    }
    if (free_buf)
        free(com->packet->data);
    com->packet->size = newsize;
    com->packet->data = decompressed;
}