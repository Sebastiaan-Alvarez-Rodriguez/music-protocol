#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#include "communication/constants/constants.h"
#include "communication/flags/flags.h"
#include "server/client_info/client_info.h"
#include "server/server/task/task.h"
#include "server/server/receive/receive.h"
#include "server/server/receive/client_search.h"
#include "stats/stats.h"
#include "receive.h"

enum check_output_flag {
    CHECK_OUTPUT_OK,
    CHECK_OUTPUT_REJECT,
    CHECK_OUTPUT_IGNORE
};

// Receives a message from the client and registers the client if it is
// a new connection, otherwise points to the current connected client
static enum check_output_flag receive_and_check(server_t* const server, com_t* receive, client_info_t** current) {
    enum recv_flag flag = com_receive(receive);
    if (flag != RECV_OK) {
        if (flag == RECV_FAULTY) {
            receive->packet->size = constants_packets_size();
            com_consume_packet(receive);
        }
        return CHECK_OUTPUT_IGNORE;
    }

    client_info_t* client = NULL;

    switch (search_client(server, receive->address, &client)) {
        case MATCH_UNUSED:
            client_info_init(client, receive, server->mf->samples);
            client_info_set_timeout(client, 3000);
            break;
        case MATCH:
            client_info_set_timeout(client, 3000);
            break;
        case NO_MATCH:
            *current = NULL;
            free(receive->packet->data);
            return CHECK_OUTPUT_REJECT;
    }
    *current = client;
    return CHECK_OUTPUT_OK;
}

// Processes an initial request from the client
static void process_initial(const com_t* const receive, client_info_t* const client, task_t* const task) {
    if(flags_is_ACK(receive->packet->flags)) {
        client->quality->current = *(uint8_t*) receive->packet->data;
        client->packets_per_batch = constants_batch_packets_amount(client->quality->current);
        client->music_chuck_size = constants_packets_size();
        client->stage = INITIAL;
        task->type = SEND_ACK;
    } else if(flags_is_RR(receive->packet->flags)) {
        client->stage = INTERMEDIATE;
        task->type = SEND_BATCH;
    } else if(flags_is_REJ(receive->packet->flags)) {
        client->stage = INTERMEDIATE;
        task->type = SEND_FAULTY;
    }
}

// Processes an intermediate request from the client
static void process_intermediate(com_t* const receive, client_info_t* const client, task_t* const task) {
    if(!client->in_use) {
        task->type = SEND_EOS;
    } else if(flags_is_RR(receive->packet->flags)) {
        puts("RR");
        task->type = SEND_BATCH;
    } else if(flags_is_REJ(receive->packet->flags)) {
        puts("REJ");
        task_set_faulty(task, receive->packet->size, receive->packet->data);
    } else if(flags_is_QTY(receive->packet->flags)) {
        task->type = SEND_ACK;
        client->quality->current = *(uint8_t*) receive->packet->data;
        client->packets_per_batch = constants_batch_packets_amount(client->quality->current);
    }
}

static void process_final(server_t* const server, com_t* const receive, client_info_t* const client, task_t* const task) {
    if(!client->in_use || flags_is_RR(receive->packet->flags)) {
        if(client->bytes_sent < server->mf->payload_size) {
            task->type = SEND_BATCH;
        }
        else {
            task->type = SEND_EOS;
            if(client->in_use) {
                stat_print(client->stat);
                client_info_free(client);
            }
            client->in_use = false;
        }
    } else if (flags_is_REJ(receive->packet->flags)) {
        task_set_faulty(task, receive->packet->size, receive->packet->data);
    } else if(flags_is_QTY(receive->packet->flags)) {
        task->type = SEND_ACK;
        client->quality->current = *(uint8_t*) receive->packet->data;
        client->packets_per_batch = constants_batch_packets_amount(client->quality->current);
    }
}

bool receive_from_client(server_t* const server, com_t* receive, client_info_t** current, task_t* const task) {
    client_info_t* client = NULL;
    enum check_output_flag flag = receive_and_check(server, receive, &client);
    if(flag == CHECK_OUTPUT_IGNORE) {
        return false;
    } else if (flag == CHECK_OUTPUT_REJECT) {
        task->type = SEND_EOS;
        return true;
    }
    switch (client->stage) {
        case INITIAL:
            process_initial(receive, client, task);
            break;
        case INTERMEDIATE:
            process_intermediate(receive, client, task);
            break;
        case FINAL:
            process_final(server, receive, client, task);
            break;
        default:
            return false;
    }
    free(receive->packet->data);
    packet_reset(receive->packet);
    *current = client;
    return true;
}
