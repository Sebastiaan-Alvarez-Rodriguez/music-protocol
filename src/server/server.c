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

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <getopt.h>

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
        struct wave_header *wh;
        int fd;

        void *data;
        uint32_t data_size;

        uint8_t *samples;
        uint32_t payload_size;
};

static struct wave_file wf = {0,};

/* open the wave file */
static int open_wave_file(struct wave_file *wf, const char *filename) {
    uint8_t *p;
    float playlength;
    struct stat statbuf;
    uint32_t *size;

    /* Open the file */
    wf->fd = open(filename, O_RDONLY);
    if (wf->fd < 0) {
        fprintf(stderr, "couldn't open %s\n", filename);
        return -1;
    }

    /* Get the size of the file */
    if (fstat(wf->fd, &statbuf) < 0) return -1;
    wf->data_size = statbuf.st_size;

    /* Map the file into memory */
    wf->data = mmap(0x0, wf->data_size, PROT_READ, MAP_SHARED, wf->fd, 0);
    if (wf->data == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return -1;
    }

    wf->wh = wf->data;

    /* Check whether the file is a wave file */
    if (strncmp(wf->wh->riff_id, "RIFF", 4)
            || strncmp(wf->wh->wave_id, "WAVE", 4)
            || strncmp(wf->wh->format_id, "fmt", 3)) {
        fprintf(stderr, "%s is not a valid wave file\n", filename);
        return -1;
    }

    /* Skip to actual data fragment */
    p = wf->data + wf->wh->format_size + 16 + 4;
    size = (uint32_t *) (p + 4);
    do {
        if (strncmp(p, "data", 4))
            break;

        wf->samples = p;
        wf->payload_size = *size;
        p += 8 + *size;
    } while (strncmp(p, "data", 4) && (uint32_t) (((void *) p) - wf->data) < statbuf.st_size);

    if (wf->wh->w_bits_per_sample != 16) {
        fprintf(stderr, "can't play sample with bitsize %d\n",
                        wf->wh->w_bits_per_sample);
        return -1;
    }

    playlength = (float) *size / (wf->wh->n_channels * wf->wh->n_samples_per_sec * wf->wh->w_bits_per_sample / 8);

    printf("file %s, mode %s, samplerate %lu, time %.1f sec\n",
                 filename, wf->wh->n_channels == 2 ? "Stereo" : "Mono",
                 wf->wh->n_samples_per_sec, playlength);

    return 0;
}

/* close the wave file/clean up */
static void close_wave_file(struct wave_file *wf) {
    munmap(wf->data, wf->data_size);
    close(wf->fd);
}

/* Setup sockets for listening on given port*/
int setupSocket(const unsigned short port, const unsigned maxConnect) {
    int socketFd;
    int socketOpt = 1;
    struct sockaddr_in server;
    //Create a socket and store the fd
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      perror("Socket creation");
      return -1;
    };
    //Make socket reusable
    if(setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &socketOpt, sizeof(socketOpt))) {
      perror("Socket Options");
      return -1;
    }
    //Zero byte server variable
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind the socket to the server address
    if(bind(socketFd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Socket Binding");
        return -1;
    }

    // Set socket in listen mode
    if(listen(socketFd, maxConnect) < 0) {
        perror("Socket Listen");
        return -1;
    }

    printf("Socket has port %hu\n", ntohs(server.sin_port));

    return socketFd;
}

// Accepts connections from a client to the server
// If no connections are made then the server waits during
// accept function.
int acceptFromClient(const unsigned serverFd) {
    int clientFd;
    struct sockaddr_in client;
    unsigned clientLen = sizeof(client);

    if((clientFd = accept(serverFd, (struct sockaddr*) &client,
        &clientLen)) < 0) {
        perror("Accept failure");
        return -1;
    }

    if(clientLen != sizeof(client)) {
        fputs(stderr, "Accept overwrote sockaddr_in struct");
        return -1;
    }

    printf("Client IP: %s\n", inet_ntoa(client.sin_addr));
    printf("Client Port: %hu\n", ntohs(client.sin_port));

    return clientFd;
}

//TODO Replace with udp packet.
//Reads from the client file descriptor
int readFromClient(const unsigned clientFd) {
    puts("Read from client");
    int readRet;
    char buff[256];
    bzero(buff, sizeof(buff));
    if((readRet = read(clientFd, buff, 256)) < 0) {
        perror("read");
        return -1;
    }
    puts(buff);
    return readRet;
}

//TODO remove for packets
#define MSG "Hello from server"

//TODO Replace with udp packet.
// Writes to the client file descriptor.
int writeToClient(const unsigned clientFd) {
    printf("Writing: %s to client\n", MSG);
    int writeRet;
    if((writeRet = write(clientFd, MSG, sizeof(MSG))) < 0) {
        perror("write");
        return -1;
    }
    return writeRet;
}

/* Runs a server that listens on given port for connections*/
int runServer(const int port) {
    int serverFd;
    if((serverFd = setupSocket(port, MAX_SOCKET_CONNECTION)) < 0) {
        return -1;
    }

    while(true) {
        int clientFd;
        if((clientFd = acceptFromClient(serverFd)) < 0) {
            return -1;
        }

        int connectionAlive;
        do {
            if((connectionAlive = readFromClient(clientFd)) < 0) {
                return -1;
            }
            if(connectionAlive) {
                if(writeToClient(clientFd) < 0) {
                    return -1;
                }
            }
        }
        while (connectionAlive);
        puts("Connection closed");
        close(clientFd);
    }
    close(serverFd);
    return 0;
}

static void showHelp(const char *progName) {
   puts(progName);
    puts("[-q quality] [-d debug] [-f soundfile]");
    puts(R"HERE(
    -q quality           Quality level, must be within [1-5] (higher number, higher quality)
    -d debug             If set, prints debug information
    -f soundfile         The soundfile to send to the client
)HERE");
}

int main(int argc, char **argv) {
    unsigned quality;

    char c;
    char *filename = NULL;
    const char *progName = argv[0];
    while ((c = getopt(argc, argv, "f:q:dh")) != -1){
        int x = ((c!='?'&&c!='f'&&c!='d'&&c!='h') ? atoi(optarg) : -1);

        switch (c) {
            case 'f':
                if (optarg != NULL)
                    filename = optarg;
                break;
            case 'q':
                if (x >= 1 && x <= 5)
                    quality = x;
                else {
                    puts("Your quality level must be within [1-5]");
                    exit(-1);
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
    }//while
    argc -= optind;
    argv += optind;

    if (!filename) {
        puts("Please specify a file to open with -f <name>");
        return -1;
    }

    /* Open the WAVE file */
    if (open_wave_file(&wf, filename) < 0) {
        return -1;
    }

    puts("Opened the wave file");

    /* TODO: Read and send audio data */

    /* Start sockets and wait for connections*/
    if(runServer(BIND_PORT) < 0) {
        return -1;
    }

    /* Clean up */
    close_wave_file(&wf);

    return 0;
}
