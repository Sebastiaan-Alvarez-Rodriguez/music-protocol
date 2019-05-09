#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "client/musicplayer/player.h"
#include "client/client/receive/receive.h"
#include "client/client/send/send.h"
#include "communication/com.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"
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
    } while (retry);
    do {
        retry = false;
        struct sockaddr_in* addr_in = (struct sockaddr_in*) client->sock;
        bzero(addr_in, sizeof(*addr_in));
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
    client->sock = malloc(sizeof(struct sockaddr*));
    connect_server(client, address, port);
    client->player = malloc(sizeof(player_t));
    client->quality = initial_quality;
    client->batch_nr = 0;
    client->EOS_received = false;
    bool retry;
    do {
        retry = false;
        send_initial_communication(client);
        if (!receive_ACK(client, true)) {
            if (menu_yes_no("Server has no room for more clients. Retry?"))
                retry = true;
            else
                exit(-1);
        }
    } while (retry);

    player_init(client->player, buffer_size / constants_packets_size(), constants_packets_size());
}


void client_fill_initial_buffer(client_t* const client) {
    puts("Filling initial buffer...");
    while (!client->EOS_received && buffer_free_size(client->player->buffer) >= constants_batch_packets_amount(client->quality)) {
        receive_batch(client);
        printf("%lu%c\n", (buffer_used_size(client->player->buffer)*100) / buffer_capacity(client->player->buffer), '%');
        // printf("client want batchnr: %i\n", client->batch_nr);
        // printf("%lu >= %lu: %i\n", buffer_free_size(client->player->buffer), constants_batch_packets_amount(client->quality), buffer_free_size(client->player->buffer) >= constants_batch_packets_amount(client->quality));
        // printf("Buf free: %lu\n", buffer_free_size(client->player->buffer));
        // printf("Buf used: %lu\n", buffer_used_size(client->player->buffer));
        // printf("Buf  cap: %lu\n", buffer_capacity(client->player->buffer));
        // sleep(3);
    }
    printf("client EOS received: %i\n", client->EOS_received);
}

void client_free(client_t* const client) {
    free(client->sock);

    player_free(client->player);
    free(client->player);
}