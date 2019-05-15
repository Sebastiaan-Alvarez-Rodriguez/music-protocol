#include <netinet/in.h>

#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "receive.h"

static void receive_correct(client_t* const client, uint8_t* buf, const size_t init_num_faulty, uint16_t* init_faulties) {
    bool all_success = true;

    size_t num_faulty = init_num_faulty;
    uint16_t* faulty_queue = init_faulties;
    do {
        send_REJ(client, num_faulty, faulty_queue);
        faulty_queue = init_faulties;
        num_faulty = 0;
        for(unsigned i = 0; i < num_faulty; ++i) {
            com_t com;
            com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
            if (!com_receive(&com)) {
                all_success = false;
                *faulty_queue = com.packet->nr;
                ++faulty_queue;
                ++num_faulty;
                continue;
            }
            else {
                uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
                memcpy(buf_ptr, com.packet->data, constants_packets_size());
            }
            com_free(&com);
        }
    }
    while(!all_success);
}

void receive_batch(client_t* const client) {
    // Request batch
    send_RR(client);
    if (receive_EOS(client, false)) {
        client->EOS_received = true;
        return;
    }
    uint16_t* faulty_queue = calloc(sizeof(uint16_t),constants_batch_packets_amount(client->quality));
    size_t num_faulty = 0;

    // Receive batch and temporarily store in buf
    uint8_t* buf = malloc(constants_batch_packets_amount(client->quality) * constants_packets_size());
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality); ++i) {
        com_t com;
        com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
        if (!com_receive(&com)) {
            *faulty_queue = com.packet->nr;
            ++faulty_queue;
            ++num_faulty;
        }
        else {
            uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
            memcpy(buf_ptr, com.packet->data, com.packet->size);
        }
        free(com.packet->data);
        com_free(&com);
    }

    if(num_faulty > 0)
      receive_correct(client, buf, num_faulty, faulty_queue);

    // Place received data in player buffer
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality); ++i) {
        uint8_t* buf_ptr = buf + i * constants_packets_size();
        buffer_add(client->player->buffer, buf_ptr, true);
    }
    free(faulty_queue);
    free(buf);
    // batch received with success. Next time, ask next batch
    client->batch_nr++;
}

bool receive_ACK(const client_t* const client, bool consume) {
    com_t com;
    printf("%p\n", (void*)client->sock);
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    com_receive(&com);
    bool is_ACK = flags_is_ACK(com.packet->flags);
    free(com.packet->data);
    com_free(&com);
    return is_ACK;
}

bool receive_EOS(const client_t* const client, bool consume) {
    com_t com;
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    com_receive(&com);
    bool is_EOS = flags_is_EOS(com.packet->flags);
    free(com.packet->data);
    com_free(&com);
    return is_EOS;
}
