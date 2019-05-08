 #include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "communication/flags/flags.h"
#include "server/music/music.h"
#include "receive/receive.h"
#include "send/send.h"

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

bool server_set_num_clients(server_t* const server, const unsigned max_clients) {
    server->max_clients = max_clients;
    server->clients = calloc(max_clients, sizeof(client_info_t));
    if(server->clients == NULL || errno == ENOMEM)
        return false;
    return true;
}

void server_run(server_t* const server) {
    bool running = true;
    struct sockaddr_in client;
    while(running) {
        com_t receive;
        client_info_t* current_client = NULL;
        receive_from_client(server, &receive, &current_client);
        puts("");
        if(current_client != NULL)
            send_to_client(server, &receive, current_client);
        com_free(&receive);
    }
}

void server_free(server_t* const server) {
    if (server->mf != NULL)
        music_free(server->mf);
    free(server->mf);

    close(server->fd);
}

void print_clients(const server_t* const server) {
    puts("Printing all registered clients:");
    for(unsigned i = 0; i < server->max_clients; ++i) {
        printf("Client[%u] ", i);
        print_client_info(&server->clients[i]);
        puts("");
    }
}
