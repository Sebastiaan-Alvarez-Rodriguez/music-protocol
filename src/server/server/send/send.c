#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "send.h"
#include "communication/flags/flags.h"
#include "server/client_info/client_info.h"

static bool prepare_packet(server_t* const server, com_t* const send, client_info_t* const client, size_t bytes_to_send, const size_t packet_nr) {
    uint8_t* music_ptr = get_music_chunk(client, packet_nr);
    if(music_ptr == NULL)
        return false;

    send->packet->data = music_ptr;
    send->packet->size = bytes_to_send;
    send->packet->nr = packet_nr;
    client->bytes_sent += bytes_to_send;

    printf("Bytes sent: %lu\n", bytes_to_send);
    printf("Total bytes sent: %u\n", client->bytes_sent);
    printf("WAV file size: %u\n", server->mf->payload_size);
    return true;
}


static bool send_intermediate(server_t* const server, com_t* const send, client_info_t* const client, const size_t packet_nr) {
    return prepare_packet(server, send, client, client->music_chuck_size, packet_nr);
}

static bool send_final(server_t* const server, com_t* const send, client_info_t* const client, const size_t packet_nr) {
    size_t bytes_to_send = client->music_chuck_size;
    if(client->bytes_sent + client->music_chuck_size > server->mf->payload_size) {
        printf("%u + %lu > %u\n", client->bytes_sent, client->music_chuck_size, server->mf->payload_size);
        bytes_to_send = server->mf->payload_size - client->bytes_sent;
    }
    if(!client->in_use)
        send->packet->flags |= FLAG_EOS;
    return prepare_packet(server, send, client, bytes_to_send, packet_nr);
}

static bool send_packet(server_t* const server, com_t* const send, client_info_t* const current, const unsigned packet_nr) {
    switch(current->stage) {
        case INTERMEDIATE:
            if(!send_intermediate(server, send, current, packet_nr))
                return false;
            break;
        case FINAL:
            if(!send_final(server, send, current, packet_nr))
                return false;
            break;
        default:
            errno = EINVAL;
            return false;
    }

    if(!com_send(send))
        return false;
    puts("");
    return true;
}

static bool send_batch(server_t* const server, com_t* const send, client_info_t* const current) {
    bool retval = true;
    for(unsigned i = 0; i < current->packets_per_batch; ++i) {
        retval &= send_packet(server, send, current, i);
    }
    return true;
}

static bool resend_faulty(server_t* const server, com_t* const receive, com_t* const send, client_info_t* const current) {
    uint16_t* faulty_ptr = (uint16_t*) receive->packet->data;
    bool retval = true;
    for(unsigned i = 0; i < current->packets_per_batch && faulty_ptr; ++i) {
        retval &= send_packet(server, send, current, *faulty_ptr);
        faulty_ptr++;
    }
    return true;
}

bool send_to_client(server_t* const server, com_t* const receive, client_info_t* const current) {
    com_t send;
    com_init(&send, server->fd, MSG_CONFIRM, (struct sockaddr*) &current->client_addr, 0, 0);

    bool retval = false;
    if(!current->resend_packets)
        retval = send_batch(server, &send, current);
    else
        retval = resend_faulty(server, receive, &send, current);

    com_free(&send);
    return retval;
}
