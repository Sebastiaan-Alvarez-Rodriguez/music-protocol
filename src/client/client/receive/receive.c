#include <netinet/in.h>

#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "receive.h"

void receive_batch(client_t* const client) {
    // Request batch
    send_RR(client);
    if (receive_EOS(client, false))
        client->EOS_received = true;
    return;
    // Receive batch and temporarily store in buf
    uint8_t* buf = malloc(constants_batch_packets_amount(client->quality) * constants_packets_size());
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality); ++i) {
        com_t com;
        com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
        if (!com_receive(&com)) {
            //TODO: do something with timeout/faulty packets
        }

        uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
        memcpy(buf_ptr, com.packet->data, constants_packets_size());

        com_free(&com);
    }

    // Place received data in player buffer
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality); ++i) {
        uint8_t* buf_ptr = buf + i * constants_packets_size();
        buffer_add(client->player->buffer, buf_ptr, constants_packets_size());
    }
    free(buf);
    // batch received with success. Next time, ask next batch
    client->batch_nr++;
}

bool receive_ACK(const client_t* const client, bool consume) {
    com_t com;
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    com_receive(&com);
    return flags_is_ACK(com.flags);
}

bool receive_EOS(const client_t* const client, bool consume) {
    com_t com;
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    com_receive(&com);
    return flags_is_EOS(com.flags);
}