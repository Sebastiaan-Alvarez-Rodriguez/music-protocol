#include <netinet/in.h>
#include <stdbool.h>

#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "receive.h"

static void receive_correct(client_t* const client, uint8_t* buf, const size_t init_num_faulty, const size_t batch_size, bool* corrects) {
    bool all_success = true;
    printf("receive correct: %lu", init_num_faulty);
    size_t num_faulty = init_num_faulty;
    uint16_t* faulty_queue = calloc(sizeof(uint16_t), init_num_faulty);
    uint16_t* ptr = faulty_queue;

    do {
        for(unsigned i = 0; i < batch_size; ++i) {
            if(!corrects[i]) {
                *ptr = i;
                ++ptr;
                printf("faulties: %u\n", i);
            }
        }
        send_REJ(client, num_faulty, faulty_queue);
        size_t count_faulty = 0;
        num_faulty = 0;
        ptr = faulty_queue;
        for(unsigned i = 0; i < num_faulty; ++i) {
            com_t com;
            com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
            if (!com_receive(&com)) {
                all_success = false;
                ++count_faulty;
            }
            else {
                corrects[com.packet->nr] = true;
                uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
                memcpy(buf_ptr, com.packet->data, constants_packets_size());
            }
            com_free(&com);
        }
    }
    while(!all_success);
    free(faulty_queue);
}

void receive_batch(client_t* const client) {
    // Request batch
    send_RR(client);
    if (receive_EOS(client, false)) {
        client->EOS_received = true;
        return;
    }
    size_t num_batch_packets = constants_batch_packets_amount(client->quality);
    size_t num_faulty = 0;
    bool package_correct[num_batch_packets];
    memset(package_correct, 0, sizeof(bool) * num_batch_packets);

    // Receive batch and temporarily store in buf
    uint8_t* buf = malloc(num_batch_packets * constants_packets_size());
    for (unsigned i = 0; i < num_batch_packets; ++i) {
        com_t com;
        com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
        if (!com_receive(&com)) {
            printf("failed [%u]\n", i);
            ++num_faulty;
        }
        else {
            package_correct[com.packet->nr] = true;
            uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
            memcpy(buf_ptr, com.packet->data, com.packet->size);
        }
        free(com.packet->data);
        com_free(&com);
    }

    printf("faulties: %lu\n", num_faulty);
    if(num_faulty > 0)
      receive_correct(client, buf, num_faulty, num_batch_packets, package_correct);

    // Place received data in player buffer
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality); ++i) {
        uint8_t* buf_ptr = buf + i * constants_packets_size();
        buffer_add(client->player->buffer, buf_ptr, true);
    }
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
