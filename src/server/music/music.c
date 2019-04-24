#include <stdbool.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "music.h"

static bool music_file_open(music_file* mf, const char* const filename) {
    /* Open the file */
    mf->fd = open(filename, O_RDONLY);
    if (mf->fd < 0) {
        fprintf(stderr, "couldn't open %s\n", filename);
        return false;
    }

    struct stat statbuf;
    /* Get the size of the file */
    if (fstat(mf->fd, &statbuf) < 0)
        return false;
    mf->data_size = statbuf.st_size;

    /* Map the file into memory */
    mf->data = mmap(0x0, mf->data_size, PROT_READ, MAP_SHARED, mf->fd, 0);
    if (mf->data == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return false;
    }

    mf->mh = mf->data;

    /* Check whether the file is a wave file */
    if (strncmp(mf->mh->riff_id, "RIFF", 4)
            || strncmp(mf->mh->music_id, "WAVE", 4)
            || strncmp(mf->mh->format_id, "fmt", 3)) {
        fprintf(stderr, "%s is not a valid wave file\n", filename);
        return false;
    }


    /* Skip to actual data fragment */
    uint8_t *p = (uint8_t*) mf->data + mf->mh->format_size + 16 + 4;
    uint32_t *size = (uint32_t*) (p + 4);
    do {
        if (strncmp((char*) p, "data", 4))
            break;

        mf->samples = p;
        mf->payload_size = *size;
        p += 8 + *size;
    } while (strncmp((char*) p, "data", 4) && (uint32_t) (((uint8_t*) p) - (uint8_t*) mf->data) < statbuf.st_size);

    if (mf->mh->w_bits_per_sample != 16) {
        fprintf(stderr, "can't play sample with bitsize %d\n",
            mf->mh->w_bits_per_sample);
        return false;
    }

    float playlength = (float) *size / (mf->mh->n_channels * mf->mh->n_samples_per_sec * mf->mh->w_bits_per_sample / 8);

    printf("file %s, mode %s, samplerate %u, time %.1f sec\n",
            filename, mf->mh->n_channels == 2 ? "Stereo" : "Mono",
            mf->mh->n_samples_per_sec, playlength);

    return true;
}

static void music_file_close(music_file* mf) {
    munmap(mf->data, mf->data_size);
    close(mf->fd);
}


bool music_init(music_file* mf, const char* const file) {
    if (!music_file_open(mf, file)) {
        printf("Failed to open %s\n",file);
        return false;
    }
    printf("Opened %s\n",file);
    return true;
}

void music_free(music_file* mf) {
    music_file_close(mf);
}