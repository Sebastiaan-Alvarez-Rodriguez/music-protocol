#ifndef MUSIC
#define MUSIC

#include <stdbool.h>
#include <stdint.h>

typedef struct {
        char riff_id[4];
        uint32_t size;
        char music_id[4];
        char format_id[4];
        uint32_t format_size;
        uint16_t w_format_tag;
        uint16_t n_channels;
        uint32_t n_samples_per_sec;
        uint32_t n_avg_bytes_per_sec;
        uint16_t n_block_align;
        uint16_t w_bits_per_sample;
} music_header;

typedef struct {
        music_header* mh;
        int fd;

        void* data;
        uint32_t data_size;

        uint8_t* samples;
        uint32_t payload_size;
} music_file;

// open the wave file with given filename
// Returns true on success, otherwise false
bool music_init(music_file* mf, const char* const file);

// close the wave file
void music_free(music_file* mf);

#endif