 #include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "task/task.h"
#include "stats/stats.h"
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
    return server->clients != NULL && errno != ENOMEM;
}

void* thread_run_timeout_timers(void* args) {
    timeout_thread_args* targs = (timeout_thread_args*) args;
    client_info_t* clients = targs->clients;
    server_t* server = targs->server;
    struct timespec time = {1, 0};
    while(targs->running) {
        for(unsigned i = 0; i < server->max_clients; ++i) {
            if(clients[i].in_use) {
                clients[i].timeout_in_ms -= (time.tv_sec * 1000);
                if(clients[i].timeout_in_ms <= 0) {
                    printf("Closing client [%u]\n", i);
                    stat_print(clients[i].stat);
                    client_info_free(&clients[i]);
                    clients[i].in_use = false;
                    print_clients(server);
                }
            }
        }
        clock_nanosleep(CLOCK_REALTIME, 0, &time, NULL);
    }
    pthread_exit(0);
}

void server_run(server_t* const server) {
    bool running = true;
    struct sockaddr_in client;
    pthread_t timeout_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    timeout_thread_args targs = {server, server->clients, &running};
    pthread_create(&timeout_thread, &attr, &thread_run_timeout_timers, &targs);

    while(running) {
        com_t com;
        struct sockaddr_in address;
        task_t task;
        client_info_t* current_client = NULL;
        com_init(&com, server->fd, MSG_WAITALL, (struct sockaddr*) &address, 0, 0);
        if (receive_from_client(server, &com, &current_client, &task)) {
            send_to_client(server, &com, current_client, &task);
        }
        task_free(&task);
        com_free(&com);
    }
    pthread_join(timeout_thread, NULL);
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
