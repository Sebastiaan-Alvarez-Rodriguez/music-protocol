/*
 * Skeleton-code behorende bij het college Netwerken, opleiding Informatica,
 * Universiteit Leiden.
 */
#include "asp.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <alsa/asoundlib.h>
#include <getopt.h>

#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100
#define BLOCK_SIZE 1024
/* 1 Frame = Stereo 16 bit = 32 bit = 4kbit */
#define FRAME_SIZE 4

#define MSG "Hello from client"

// Sets up sockets to connect to a server at given
// address and port.
int connectServer(const unsigned short port, const char* address, struct sockaddr_in* out) {
    int socketFd;
    struct sockaddr_in server;

    if((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket Creation");
        return -1;
    }

    int serverLen = sizeof(server);
    bzero(&server, serverLen);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if(inet_aton(address, &server.sin_addr) == 0) {
        perror("Function inet_aton");
        return -1;
    }

    out->sin_addr.s_addr = server.sin_addr.s_addr;
    out->sin_family = server.sin_family;
    out->sin_port = server.sin_port;

    return socketFd;
}


bool debug = false;

static void showHelp(const char *prog_name) {
    puts(prog_name);
    puts("[-q quality] [-i IP address] [-p port] [-d debug] [-h]");
    puts("");
    puts("-b buffersize Give a buffer size in KB");
    puts("-q quality    Number within [1-5] (higher means more quality)");
    puts("-i IP address IP address of server");
    puts("-p port       Specify which port the client should connect to");
    puts("-d debug      If set, prints debug information");
    puts("-h            Shows this dialog");
}

int main(int argc, char **argv) {
    const char* const prog_name = argv[0];
    unsigned quality = 5;
    unsigned buffer_size = 1024;
    unsigned short bind_port = 1235;
    char* server_address = "127.0.0.1";

    char c;
    while ((c = getopt(argc, argv, "b:q:i:p:dh")) != -1){
        switch (c) {
            case 'b':
                if (*optarg >= '1') {
                    buffer_size = atoi(optarg);
                    printf("Buffersize set to %i\n", buffer_size);
                } else {
                    puts("Provide a bufferspace >= 1");
                    return -1;
                }
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
            case 'i':

            case 'p':
                bind_port = (unsigned short) atoi(optarg);
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


    int fd;
    struct sockaddr_in server;
    if((fd = connectServer(bind_port, server_address, &server)) < 0) {
        return -1;
    }
    int len = sizeof(server);
    sendto(fd, MSG, sizeof(MSG), MSG_CONFIRM, (const struct sockaddr*) &server, sizeof(server));

    puts("sent");

    char buffer[1024];
    int n;
    n = recvfrom(fd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*) &server, &len);
    buffer[n] = '\0';
    puts(buffer);

    /* Open audio device */
    snd_pcm_t *snd_handle;

    int err = snd_pcm_open(&snd_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

    if (err < 0) {
        fprintf(stderr, "couldnt open audio device: %s\n", snd_strerror(err));
        return -1;
    }

    /* Configure parameters of PCM output */
    err = snd_pcm_set_params(snd_handle,
         SND_PCM_FORMAT_S16_LE,
         SND_PCM_ACCESS_RW_INTERLEAVED,
         NUM_CHANNELS,
         SAMPLE_RATE,
         0,              // Allow software resampling
         500000);        // 0.5 seconds latency
    if (err < 0) {
        printf("couldnt configure audio device: %s\n", snd_strerror(err));
        return -1;
    }

    /* set up buffers/queues */
    uint8_t* recvbuffer = malloc(BLOCK_SIZE);
    uint8_t* playbuffer = malloc(BLOCK_SIZE);

    /* TODO: fill the buffer */

    /* Play */
    printf("playing...\n");

    int i = 0;
    unsigned blocksize = 0;
    uint8_t* play_ptr;
    uint8_t* recv_ptr = recvbuffer;
    while (true) {
        if (i <= 0) {
            /* TODO: get sample */

            play_ptr = playbuffer;
            i = blocksize;
        }

        /* write frames to ALSA */
        snd_pcm_sframes_t frames = snd_pcm_writei(snd_handle, play_ptr, (blocksize - (*play_ptr - *playbuffer)) / FRAME_SIZE);

        /* Check for errors */
        int ret = 0;
        if (frames < 0)
            ret = snd_pcm_recover(snd_handle, frames, 0);
        if (ret < 0) {
            fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %i\n", ret);
            exit(EXIT_FAILURE);
        }
        if (frames > 0 && frames < (blocksize - (*play_ptr - *playbuffer)) / FRAME_SIZE)
            printf("Short write (expected %i, wrote %li)\n",
                         (int) (blocksize - (*play_ptr - *playbuffer)) / FRAME_SIZE, frames);

        /* advance pointers accordingly */
        if (frames > 0) {
            play_ptr += frames * FRAME_SIZE;
            i -= frames * FRAME_SIZE;
        }

        if ((unsigned)(*play_ptr - *playbuffer) == blocksize)
            i = 0;


        /* TODO: try to receive a block from the server? */

    }

    close(fd);

    /* clean up */
    free(recvbuffer);
    free(playbuffer);

    snd_pcm_drain(snd_handle);
    snd_pcm_hw_free(snd_handle);
    snd_pcm_close(snd_handle);

    return 0;
}
