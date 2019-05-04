#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "send.h"
#include "server/client_info/client_info.h"

static bool send_packet(com_t* const send) {
    if(!send_com(send))
        return false;
    return true;
}

static bool prepare_packet(com_t* const send, client_info_t* const client, const unsigned packet_nr) {
    uint8_t* music_ptr = get_music_chunk(client, packet_nr);
    if(music_ptr == NULL)
        return false;

    send->packet->data = music_ptr;
    send->packet->size = client->music_chuck_size;
    send->packet->nr = packet_nr;
    client->bytes_sent += client->music_chuck_size;
    client->stage = INTERMEDIATE;

    printf("Bytes sent: %lu\n", client->music_chuck_size);
    printf("Total bytes sent: %u\n", client->bytes_sent);
    return true;
}


static bool initial_send(com_t* const send, client_info_t* const client) {
    return prepare_packet(send, client, 0);
}

static bool intermediate_send(com_t* const send, client_info_t* const client) {
    return prepare_packet(send, client, ++client->packet_nr);
}

bool send_to_client(server_t* const server, client_info_t* const current) {
    com_t send;
    com_init(&send, server->fd, MSG_CONFIRM, (struct sockaddr*) &current->client_addr, 0, 0);

    switch(current->stage) {
        case INITIAL:
            if(!initial_send(&send, current))
                return false;
            break;
        case INTERMEDIATE:
            if(!intermediate_send(&send, current))
                return false;
            break;
        case FINAL:
            break;
        default:
            errno = EINVAL;
            return false;
    }

    if(!send_packet(&send))
        return false;
    // free_com(&send);
    return true;
}
