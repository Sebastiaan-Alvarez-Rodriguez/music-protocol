#include <stdint.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "client/musicplayer/player.h"
#include "client/client/receive/receive.h"
#include "client/client/send/send.h"
#include "communication/com.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"
#include "communication/quality/quality.h"
#include "menu/menu.h"
#include "client.h"

// Sets up sockets to connect to a server at given address and port.
static void connect_server(client_t* const client, const char* address, const unsigned short port) {
    int socket_fd;

    printf("Connecting to server at %s:%u\n", address, port);
    bool retry;
    do {
        retry = false;
        if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation");
            if (menu_yes_no("Retry?"))
                retry = true;
            else
                exit(-1);
        }
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; //100.000 us = 100 ms
        if (setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("Error");
        }
    } while (retry);
    do {
        retry = false;
        struct sockaddr_in* addr_in = (struct sockaddr_in*) client->sock;
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(port);

        if(inet_aton(address, &addr_in->sin_addr) == 0) {
            perror("Function inet_aton");
            if (menu_yes_no("Retry?"))
                retry = true;
            else
                exit(-1);
        }
    } while (retry);
    client->fd = (unsigned) socket_fd;
}

void client_init(client_t* const client, const char* address, const unsigned short port, const unsigned buffer_size, const unsigned initial_quality) {
    client->sock = malloc(sizeof(struct sockaddr));
    connect_server(client, address, port);
    client->player = malloc(sizeof(player_t));
    client->quality = malloc(sizeof(quality_t));
    quality_init(client->quality, initial_quality);

    client->batch_nr = 0;
    client->EOS_received = false;
    bool retry;
    do {
        retry = false;
        send_initial_communication(client);
        enum recv_flag flag = receive_ACK(client, true);
        if (flag != RECV_OK) {
            const char* msg;
            if (flag == RECV_FAULTY)
                retry = true;
            else {
                if (flag == RECV_TIMEOUT)
                    msg = "Server connection could not be established. Retry?";
                else
                    msg = "Server has no room for more clients. Retry?";
                if (menu_yes_no(msg))
                    retry = true;
                else
                    exit(-1);
            }
        }
    } while (retry);

    player_init(client->player, buffer_size / constants_packets_size(), constants_packets_size());
}


void client_fill_initial_buffer(client_t* const client) {
    puts("Filling initial buffer...");
    do {
        receive_batch(client);
        printf("%lu%c\n", (buffer_used_size(client->player->buffer)*100) / buffer_capacity(client->player->buffer), '%');
    } while(!client->EOS_received && buffer_free_size(client->player->buffer) >= constants_batch_packets_amount(client->quality->current));
    puts("Done! Playing...");
}

void client_free(client_t* const client) {
    free(client->sock);

    player_free(client->player);
    free(client->player);
    free(client->quality);
}