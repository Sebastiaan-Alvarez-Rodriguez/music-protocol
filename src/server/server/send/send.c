#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "communication/flags/flags.h"
#include "communication/quality/quality.h"
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
    if(client->bytes_sent + client->music_chuck_size > server->mf->payload_size)
        bytes_to_send = server->mf->payload_size - client->bytes_sent;
    printf("Nr [%u], sending bytes: %lu\n", packet_nr, bytes_to_send);
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
                if (quality_suggest_downsampling(current->quality))
                    downsample(send, 8);
                if (quality_suggest_compression(current->quality))
                    compress(send);
                //TODO: ANDREW kijk
                
                break;
            case FINAL:
                prepare_final(server, send, current, i);
                break;
            default:
                errno = EINVAL;
                return false;
        }
        retval &= com_send(send);
        if (current->stage == INTERMEDIATE && (
            quality_suggest_downsampling(current->quality)
            || quality_suggest_compression(current->quality)))
            free(send->packet->data);
    }
    current->music_ptr += current->packets_per_batch * current->music_chuck_size;
    return retval;
}

static bool send_faulty(server_t* const server, com_t* const send, client_info_t* const current, const task_t* const task) {
    uint8_t* faulty_ptr = (uint8_t*) task->arg;
    bool retval = true;
    uint32_t batch_nr = *(uint32_t*) faulty_ptr;
    faulty_ptr += 4;
    uint16_t batch_size = (task->arg_size - sizeof(uint32_t) / sizeof(uint8_t));

    for(unsigned i = 0; i < batch_size && faulty_ptr; ++i) {
        switch(current->stage) {
            case INTERMEDIATE:
            //TODO: ANDREW kijk
                current->music_ptr -= current->packets_per_batch * current->music_chuck_size;
                prepare_intermediate(send, current, *faulty_ptr);
                current->music_ptr += current->packets_per_batch * current->music_chuck_size;
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

bool send_to_client(server_t* const server, com_t* const com, client_info_t* const current, const task_t* const task) {
    printf("TASK: %u\n", task->type);
    bool retval = false;
    switch (task->type) {
        case SEND_ACK:
            retval &= send_flags(com, FLAG_ACK);
            break;
        case SEND_EOS:
            puts("SEND EOS");
            retval &= send_flags(com, FLAG_EOS);
            break;
        case SEND_BATCH:
            retval &= send_batch(server, com, current);
            break;
        case SEND_FAULTY:
            retval &= send_faulty(server, com, current, task);
            break;
        default:
            return false;
    }
    return retval;
}
