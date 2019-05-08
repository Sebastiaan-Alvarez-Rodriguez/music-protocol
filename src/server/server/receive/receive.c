#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "receive.h"
#include "communication/flags/flags.h"
#include "server/server/receive/receive.h"
#include "server/server/receive/client_search.h"

// Receives a message from the client and registers the client if it is
// a new connection, otherwise points to the current connected client
static bool receive_and_check(server_t* const server, com_t* const receive, client_info_t** current) {
    struct sockaddr_in client_address;
    com_init(receive, server->fd, MSG_WAITALL, (struct sockaddr*) &client_address, 0, 0);
    if (!receive_com(receive))
        return false;

    client_info_t* client = NULL;

    switch (search_client(server, receive->address, &client)) {
        case MATCH:
            puts("welcome back");
            break;
        case MATCH_UNUSED:
            puts("new user");
            client_info_init(client, receive);
            client->music_ptr = server->mf->samples;
            print_clients(server);
            break;
        case NO_MATCH:
            puts("rejected, clients full");
            return false;
        default:
            errno = EINVAL;
            return false;
    }
    *current = client;
    return true;
}

// Processes an initial request from the client
// TODO change batchsize
static bool initial_receive(const com_t* const receive, client_info_t* const client) {
    puts("initial_receive");
    if(!flags_is_ACK(receive->packet->flags))
        return false;

    size_t* client_buff_size;
    client_buff_size = (size_t*) receive->packet->data;
    *client_buff_size *= 1000;

    client->buffer_size = *client_buff_size;
    client->packets_per_batch = 32;
    client->music_chuck_size = calculate_packet_size(*client_buff_size, client->packets_per_batch * 64);
    client->stage = INTERMEDIATE;

    printf("Client buffer size: %lu\n", *client_buff_size);
    printf("Client packet size: %lu\n", client->music_chuck_size);
    printf("Client packets per batch: %lu\n", client->packets_per_batch);

    return true;
}

// Processes an intermediate request from the client
// TODO change batchsize
static bool intermediate_receive(server_t* const server, com_t* const receive, client_info_t* const client) {
    if(flags_is_RR(receive->packet->flags)) {
        client->music_ptr += client->packets_per_batch * client->music_chuck_size;
        client->packets_per_batch = 32;
        if(client->bytes_sent + (client->packets_per_batch * client->music_chuck_size) >= server->mf->payload_size)
            client->stage = FINAL;
    }
    else if(flags_is_REJ(receive->packet->flags)) {
        client->resend_packets = true;
        client->packets_per_batch = receive->packet->size / sizeof(uint16_t);
    }
    else
        return false;
    return true;
}

static void final_receive(com_t* const receive, client_info_t* const client) {
    if(flags_is_RR(receive->packet->flags)) {
        client->in_use = false;
    }
    else if (flags_is_REJ(receive->packet->flags)) {
        client->resend_packets = true;
        client->packets_per_batch = receive->packet->size / sizeof(uint16_t);
    }
}

bool receive_from_client(server_t* const server, com_t* const receive, client_info_t** current) {
    client_info_t* client = NULL;
    if(!receive_and_check(server, receive, &client))
        return false;

    switch (client->stage) {
        case INITIAL:
            if(!initial_receive(receive, client))
                return false;
            break;
        case INTERMEDIATE:
            if(!intermediate_receive(server, receive, client))
                return false;
            break;
        case FINAL:
            final_receive(receive, client);
            break;
        default:
            return false;
    }
    *current = client;
    return true;
}
