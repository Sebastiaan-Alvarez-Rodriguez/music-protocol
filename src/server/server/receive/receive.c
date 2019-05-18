#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#include "communication/constants/constants.h"
#include "communication/flags/flags.h"
#include "server/client_info/client_info.h"
#include "server/server/receive/receive.h"
#include "server/server/receive/client_search.h"
#include "receive.h"

// Receives a message from the client and registers the client if it is
// a new connection, otherwise points to the current connected client
static bool receive_and_check(server_t* const server, com_t* receive, client_info_t** current) {
    if (com_receive(receive) != RECV_OK)
        return false;

    client_info_t* client = NULL;

    switch (search_client(server, receive->address, &client)) {
        case MATCH:
            puts("welcome back");
            break;
        case MATCH_UNUSED:
            puts("new user");
            client_info_init(client, receive, server->mf->samples);
            print_clients(server);
            break;
        case NO_MATCH:
            puts("rejected, clients full");
            *current = NULL;
            return false;
        default:
            errno = EINVAL;
            return false;
    }
    *current = client;
    return true;
}

// Processes an initial request from the client
static bool process_initial(const com_t* const receive, client_info_t* const client, task_t* const task) {
    puts("process_initial");
    bool retval = false;
    if(flags_is_ACK(receive->packet->flags)) {
        client->quality->current = *(uint8_t*) receive->packet->data;
        client->packets_per_batch = constants_batch_packets_amount(client->quality->current);
        client->music_chuck_size = constants_packets_size();
        client->stage = INITIAL;
        task->type = SEND_ACK;

        printf("Client packet size: %lu\n", client->music_chuck_size);
        printf("Client packets per batch: %lu\n", client->packets_per_batch);
        retval = true;
    } else if(flags_is_RR(receive->packet->flags)) {
        client->stage = INTERMEDIATE;
        task->type = SEND_BATCH;
        retval = true;
    }
    return retval;
}

// Processes an intermediate request from the client
static void process_intermediate(server_t* const server, com_t* const receive, client_info_t* const client, task_t* const task) {
    if(!client->in_use) {
        task->type = SEND_EOS;
    } else if(flags_is_RR(receive->packet->flags)) {
        task->type = SEND_BATCH;
        client->music_ptr += client->packets_per_batch * client->music_chuck_size;
        client->packets_per_batch = constants_batch_packets_amount(client->quality->current);
        puts("RR\n");
        printf("Bytes sent: %u\n", client->bytes_sent);
        printf("Total Bytes: %u\n", server->mf->payload_size);
        printf("Batch size: %lu\n", client->packets_per_batch * client->music_chuck_size);
        if(client->bytes_sent + (client->packets_per_batch * client->music_chuck_size) >= server->mf->payload_size)
            client->stage = FINAL;
    } else if(flags_is_REJ(receive->packet->flags)) {
        task->type = SEND_FAULTY;
        task->arg = receive->packet->data;
    } else if(flags_is_QTY(receive->packet->flags)) {
        task->type = SEND_ACK;
        client->quality->current = *(uint8_t*) receive->packet->data;
    }
}

static void process_final(com_t* const receive, client_info_t* const client, task_t* const task) {
    if(!client->in_use || flags_is_RR(receive->packet->flags)) {
        task->type = SEND_EOS;
        client->in_use = false;
        client_info_free(client);
        printf("Client in use: %s\n", client->in_use ? "TRUE" : "FALSE");
        puts("dd");
    } else if (flags_is_REJ(receive->packet->flags)) {
        task->type = SEND_FAULTY;
        task->arg = receive->packet->data;
    }
}

bool receive_from_client(server_t* const server, com_t* receive, client_info_t** current, task_t* const task) {
    client_info_t* client = NULL;
    if(!receive_and_check(server, receive, &client)) {
        task->type = SEND_EOS;
        return false;
    }
    switch (client->stage) {
        case INITIAL:
            if(!process_initial(receive, client, task))
                return false;
            break;
        case INTERMEDIATE:
            process_intermediate(server, receive, client, task);
            break;
        case FINAL:
            process_final(receive, client, task);
            break;
        default:
            return false;
    }
    printf("Client in use: %s\n", client->in_use ? "TRUE" : "FALSE");
    *current = client;
    return true;
}
