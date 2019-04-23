/*
 * Skeleton-code behorende bij het college Netwerken, opleiding Informatica,
 * Universiteit Leiden.
 */

#include "asp.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <getopt.h>

#include "communication/com.h"
#include "communication/flags/flags.h"

#define MAX_SOCKET_CONNECTION 3
#define BIND_PORT 1235

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
        struct wave_header* wh;
        int fd;

        void* data;
        uint32_t data_size;

        uint8_t* samples;
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
static void close_wave_file(struct wave_file* wf) {
    munmap(wf->data, wf->data_size);
    close(wf->fd);
}

/* Setup sockets for listening on given port*/
int setupSocket(const unsigned short port) {
    int socketFd;
    int socketOpt = 1;
    struct sockaddr_in server;
    //Create a socket and store the fd
    if((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    };

    struct timeval time;
    time.tv_sec = 5;
    time.tv_usec = 0;
    if(setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time))){
        perror("setsockopt");
        return -1;
    };

    //Zero byte server variable
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind the socket to the server address
    if(bind(socketFd, (const struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind");
        return -1;
    }

    printf("Socket has port %hu\n", ntohs(server.sin_port));
    return socketFd;
}

/* Runs a server that listens on given port for connections*/
int runServer(const unsigned port, const unsigned maxConnections) {
    int sockfd;
    if((sockfd = setupSocket(port)) < 0) {
        return -1;
    }



    while(true) {
        com_t com;
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        init_com(&com, sockfd, MSG_WAITALL, (struct sockaddr*) &client, FLAG_NONE);

        /*TODO send stuff to client*/
        if(receive_com(&com)) {


            char* received = malloc(com.packet->size);
            memcpy(received, com.packet->data, com.packet->size);
            printf("Received: %s\n", received);
            puts("Connection closed");
            free(received);
            free_com(&com);
            sleep(10);
        }
        else {
            if(errno == EWOULDBLOCK) {
                puts("Connection timeout");
            }
        }
    }

    return sockfd;
}

static void showHelp(const char *prog_name) {
    puts(prog_name);
    puts("[-q quality] [-f soundfile] [-p port] [-d debug] [-h]");
    puts("");
    puts("-q quality    Number within [1-5] (higher means more quality)");
    puts("-f soundfile  The soundfile to send to the client");
    puts("-p port       Specify which port the client should connect to");
    puts("-d debug      If set, prints debug information");
    puts("-h            Shows this dialog");
}

int main(int argc, char** argv) {
    unsigned quality = 5;
    unsigned short bind_port = 1235;
    char c;
    char* filename = NULL;
    const char* const prog_name = argv[0];
    while ((c = getopt(argc, argv, "f:q:dh")) != -1){
        switch (c) {
            case 'f':
                if (optarg != NULL)
                    filename = optarg;
                break;
            case 'p':
                bind_port = (unsigned short) atoi(optarg);
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
                showHelp(prog_name);
                return 0;
        }
    }
    argc -= optind;
    argv += optind;

    if (!filename) {
        puts("  Please specify a sound file to open with -f <name>");
        return -1;
    }

    /* Open the WAVE file */
    if (!open_wave_file(&wf, filename))
        return -1;
    printf("Opened %s\n",filename);

    /* TODO: Read and send audio data */

    int sockfd;
    /* Start sockets and wait for connections*/
    if((sockfd = runServer(bind_port, 10)) < 0) {
        return -1;
    }

    close(sockfd);
    /* Clean up */
    close_wave_file(&wf);

    return 0;
}
