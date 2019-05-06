#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "client/musicplayer/player.h"
#include "communication/com.h"
#include "communication/flags/flags.h"
#include "menu/menu.h"
#include "client.h"

// Sets up sockets to connect to a server at given address and port.
static void connect_server(client_t* const client, const char* address, const unsigned short port) {
    int socket_fd;

    printf("Connecting to server at %s:%u\n", address, port);
    while (true) {
        if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation");
            if (menu_yes_no("Retry?"))
                continue;
            else
                exit(-1);
        }

        struct sockaddr_in* addr_in = (struct sockaddr_in*) client->sock;
        bzero(addr_in, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(port);

        if(inet_aton(address, &addr_in->sin_addr) == 0) {
            perror("Function inet_aton");
            if (menu_yes_no("Retry?"))
                continue;
            else
                exit(-1);
        }
    }
    client->fd = (unsigned) socket_fd;
}

void client_init(client_t* const client, const char* address, const unsigned short port) {
    client->sock = malloc(sizeof(struct sockaddr*));
    connect_server(client, address, port);
    client->player = malloc(sizeof(player_t));
    // TODO: change hardcoded max_elems=50, elem_size=1024 (bytes)
    player_init(client->player, 50, 1024);
}

bool send_initial_comunication(const client_t* const client, const uint16_t buffersize) {
    com_t com;
    com_init(&com, client->fd, MSG_CONFIRM, client->sock, flags_get_raw(1, FLAG_ACK), 0);
    com.packet->data = malloc(sizeof(uint16_t));
    if (com.packet->data == NULL || errno == ENOMEM)
        return false;
    com.packet->size = sizeof(uint16_t);
    memcpy(com.packet->data, &buffersize, sizeof(uint16_t));
    send_com(&com);
    free_com(&com);
    
    com_init(&com, client->fd, MSG_WAITALL, client->sock, flags_get_raw(1, FLAG_NONE), 0);
    receive_peek_com(&com);
    printf("Received an initial packet!");
    free_com(&com);
    return true;
}


void client_fill_initial_buffer(client_t* const client) {
    // // Quality
    // while (client.player.buffer->buf_free_size() > sizeof(packet))
    //     // Request een batch

}

void client_free(client_t* const client) {
    free(client->sock);

    player_free(client->player);
    free(client->player);
}