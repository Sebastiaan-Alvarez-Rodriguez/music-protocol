#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server/music/music.h"
#include "server.h"


// LEGACY CODE BELOW
// /* Runs a server that listens on given port for connections*/
// int runServer(const server_t* const server) {

//     while(true) {
//         com_t com;
//         struct sockaddr_in client;
//         bzero(&client, sizeof(client));
//         com_init(&com, server->portsockfd, MSG_WAITALL, (struct sockaddr*) &client, FLAG_NONE, 0);

//         //TODO send stuff to client
//         receive_com(&com);
        
//         char* received = malloc(com.packet->size);
//         memcpy(received, com.packet->data, com.packet->size);
//         printf("Received: %s\n", received);
//         puts("Connection closed");
//         free(received);
//         free_com(&com);
//         sleep(10);
//     }

//     return sockfd;
// }

void server_init(server_t* const server) {
    server->port = 0;
    server->mf = NULL;
}

bool server_set_music(server_t* const server, const char* const musicfile) {
    server->mf = malloc(sizeof(music_file));
    if (server->mf == NULL || errno == ENOMEM)
        return false;
    return music_init(server->mf, musicfile);
}

bool server_set_port(server_t* const server, unsigned short port) {
    //Create a socket and store the fd
    int tmpfd;
    if((tmpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("Socket creation");
      return false;
    };
    server->fd = tmpfd;
    struct sockaddr_in sock;
    bzero((char *) &sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = INADDR_ANY;
    sock.sin_port = htons(port);

    // Bind the socket to the server address
    if(bind(server->fd, (const struct sockaddr*) &sock, sizeof(sock)) < 0) {
        perror("Socket Binding");
        return false;
    }
    printf("Socket has port %hu\n", ntohs(sock.sin_port));
    return true;
}

void server_run(server_t* const server, unsigned initial_quality) {
    // TODO: Read and send audio data to multiple clients
}

void server_free(server_t* const server) {
    if (server->mf != NULL)
        music_free(server->mf);
    free(server->mf);

    close(server->fd);
}