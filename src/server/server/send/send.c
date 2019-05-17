#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "communication/flags/flags.h"
#include "compression/compress.h"
#include "server/client_info/client_info.h"

#include "send.h"

static void prepare_music_packet(com_t* const send, client_info_t* const client, size_t bytes_to_send, const size_t packet_nr) {
    send->packet->data = get_music_chunk(client, packet_nr);
    send->packet->size = bytes_to_send;
    send->packet->nr = packet_nr;
    client->bytes_sent += bytes_to_send;
}

static void prepare_intermediate(com_t* const send, client_info_t* const client, const uint16_t packet_nr) {
    prepare_music_packet(send, client, client->music_chuck_size, packet_nr);
}

static void prepare_final(server_t* const server, com_t* const send, client_info_t* const client, const uint16_t packet_nr) {
    size_t bytes_to_send = client->music_chuck_size;
    if(client->bytes_sent + client->music_chuck_size > server->mf->payload_size) {
        // printf("%u + %lu > %u\n", client->bytes_sent, client->music_chuck_size, server->mf->payload_size);
        bytes_to_send = server->mf->payload_size - client->bytes_sent;
    }
    // printf("Nr [%u], sending bytes: %lu\n", packet_nr, bytes_to_send);
    prepare_music_packet(send, client, bytes_to_send, packet_nr);
}

static bool send_flags(com_t* const send, const uint8_t flags) {
    send->packet->flags = flags;
    send->packet->nr = 0;
    send->packet->size = 0;
    return com_send(send);
}

static bool send_batch(server_t* const server, com_t* const send, client_info_t* const current) {
    bool retval = true;
    for(unsigned i = 0; i < current->packets_per_batch; ++i) {
        switch(current->stage) {
            case INTERMEDIATE:
                prepare_intermediate(send, current, i);
                if (current->current_q_level == 1)
                    downsample(send, 8);
                if (current->current_q_level <= 2)
                    compress(send);
                break;
            case FINAL:
                prepare_final(server, send, current, i);
                break;
            default:
                errno = EINVAL;
                return false;
        }
        retval &= com_send(send);
        if (current->current_q_level <= 2 && current->stage == INTERMEDIATE)
            free(send->packet->data);
    }
    return retval;
}

static bool send_faulty(server_t* const server, com_t* const send, client_info_t* const current, const task_t* const task) {
    uint16_t* faulty_ptr = (uint16_t*) task->arg;
    bool retval = true;
    uint32_t batch_nr = *(uint32_t*) faulty_ptr;
    faulty_ptr += 2;
    uint16_t batch_size = (task->arg_size - sizeof(uint32_t) / sizeof(uint16_t));

    for(unsigned i = 0; i < batch_size && faulty_ptr; ++i) {
        switch(current->stage) {
            case INTERMEDIATE:
                prepare_intermediate(send, current, *faulty_ptr);
                break;
            case FINAL:
                prepare_final(server, send, current, *faulty_ptr);
                break;
            default:
                errno = EINVAL;
                return false;
        }
        retval &= com_send(send);
        faulty_ptr++;
    }
    return retval;
}

bool send_to_client(server_t* const server, client_info_t* const current, const task_t* const task) {
    com_t send;
    com_init(&send, server->fd, MSG_CONFIRM, (struct sockaddr*) &current->client_addr, 0, 0);
    if(current->stage == FINAL)
        puts("FINAL STAGE\n\n\n");
    bool retval = false;
    switch (task->type) {
        case SEND_ACK:
            retval &= send_flags(&send, FLAG_ACK);
            break;
        case SEND_EOS:
            retval &= send_flags(&send, FLAG_EOS);
            break;
        case SEND_BATCH:
            retval &= send_batch(server, &send, current);
            break;
        case SEND_FAULTY:
            retval &= send_faulty(server, &send, current, task);
            break;
        default:
            return false;
    }

    com_free(&send);
    return retval;
}
