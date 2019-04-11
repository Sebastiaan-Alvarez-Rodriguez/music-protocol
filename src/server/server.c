/*
 * Skeleton-code behorende bij het college Netwerken, opleiding Informatica,
 * Universiteit Leiden.
 */

#include "asp.h"
#include "buffer/buffer.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netinet/in.h>

#include <getopt.h>

static int asp_socket_fd = -1;

bool debug = false;


struct wave_header {
        char riff_id[4];
        uint32_t size;
        char wave_id[4];
        char format_id[4];
        uint32_t format_size;
        uint16_t w_format_tag;
        uint16_t n_channels;
        uint32_t n_samples_per_sec;
        uint32_t n_avg_bytes_per_sec;
        uint16_t n_block_align;
        uint16_t w_bits_per_sample;
};

/* wave file handle */
struct wave_file {
        struct wave_header *wh;
        int fd;

        void *data;
        uint32_t data_size;

        uint8_t *samples;
        uint32_t payload_size;
};

static struct wave_file wf = {0,};

// open the wave file with given filename
// Returns true on success, otherwise false
static int open_wave_file(struct wave_file *wf, const char *filename) {
    /* Open the file */
    wf->fd = open(filename, O_RDONLY);
    if (wf->fd < 0) {
        fprintf(stderr, "couldn't open %s\n", filename);
        return false;
    }

    struct stat statbuf;
    /* Get the size of the file */
    if (fstat(wf->fd, &statbuf) < 0)
        return false;
    wf->data_size = statbuf.st_size;

    /* Map the file into memory */
    wf->data = mmap(0x0, wf->data_size, PROT_READ, MAP_SHARED, wf->fd, 0);
    if (wf->data == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return false;
    }

    wf->wh = wf->data;

    /* Check whether the file is a wave file */
    if (strncmp(wf->wh->riff_id, "RIFF", 4)
            || strncmp(wf->wh->wave_id, "WAVE", 4)
            || strncmp(wf->wh->format_id, "fmt", 3)) {
        fprintf(stderr, "%s is not a valid wave file\n", filename);
        return false;
    }


    /* Skip to actual data fragment */
    uint8_t *p = (uint8_t*) wf->data + wf->wh->format_size + 16 + 4;
    uint32_t *size = (uint32_t*) (p + 4);
    do {
        if (strncmp((char*) p, "data", 4))
            break;

        wf->samples = p;
        wf->payload_size = *size;
        p += 8 + *size;
    } while (strncmp((char*) p, "data", 4) && (uint32_t) (((uint8_t*) p) - (uint8_t*) wf->data) < statbuf.st_size);

    if (wf->wh->w_bits_per_sample != 16) {
        fprintf(stderr, "can't play sample with bitsize %d\n",
            wf->wh->w_bits_per_sample);
        return false;
    }

    float playlength = (float) *size / (wf->wh->n_channels * wf->wh->n_samples_per_sec * wf->wh->w_bits_per_sample / 8);

    printf("file %s, mode %s, samplerate %u, time %.1f sec\n",
            filename, wf->wh->n_channels == 2 ? "Stereo" : "Mono",
            wf->wh->n_samples_per_sec, playlength);

    return true;
}

/* close the wave file/clean up */
static void close_wave_file(struct wave_file *wf) {
    munmap(wf->data, wf->data_size);
    close(wf->fd);
}

static void showHelp(const char *progName) {
    puts(progName);
    puts("[-q quality] [-d debug] [-f soundfile]");
    puts("");
    puts("-q quality    Number within [1-5] (higher means more quality)");
    puts("-d debug      If set, prints debug information");
    puts("-f soundfile  The soundfile to send to the client");
}

void buffertest(void) {
    size_t bufsize = 4;
    buffer buf;
    if (!buf_init(&buf, bufsize, sizeof(int)))
        puts("Did not make buf");
    printf("Buffer made at address: %p\n", (void*)&buf);

    int x = 1;
    buf_add(&buf, &x, false);
    x++;
    buf_add(&buf, &x, false);
    x++;
    buf_add(&buf, &x, false);
    x++;
    buf_add(&buf, &x, false);
    x++;
    buf_add(&buf, &x, true);
    x++;

    size_t used = buf_used_size(&buf);
    printf("Used size: %li\n", used);
    for (size_t i = 0; i < used; i++)
        printf("Read index: %li, %i\n", i, *(int*)buf_read(&buf, i));
    buf_free(&buf);
}

int main(int argc, char **argv) {
    unsigned quality;

    char c;
    char *filename = NULL;
    const char *progName = argv[0];
    while ((c = getopt(argc, argv, "f:q:dh")) != -1){
        switch (c) {
            case 'f':
                if (optarg != NULL)
                    filename = optarg;
                break;
            case 'q':
                if (*optarg >= '1' && *optarg <= '5') {
                    quality = atoi(optarg);
                    printf("Quality set to %i\n", quality);
                }
                else {
                    puts("Your quality level must be within [1-5]");
                    return -1;
                }
                break;
            case 'd':
                debug = true;
                break;
            case 'h':
            default:
                showHelp(progName);
                return -1;
        }
    }
    argc -= optind;
    argv += optind;

    buffertest();

    if (!filename) {
        puts("Please specify a file to open with -f <name>");
        return -1;
    }

    /* Open the WAVE file */
    if (!open_wave_file(&wf, filename))
        return -1;
    printf("Opened %s\n",filename);

    /* TODO: Read and send audio data */

    /* Clean up */
    close_wave_file(&wf);

    return 0;
}
