#include <netinet/in.h>
#include <stdbool.h>

#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "compression/compress.h"

#include "receive.h"

void print_array(bool* arr, size_t size) {
    printf("[");
    for(unsigned i = 0; i < size; ++i) {
        if(i % 32 == 0 && (i != 0 && i != size-1))
            printf("%d,\n", arr[i]);
        else
            printf("%d,", arr[i]);
    }
    printf("]\n");
}

void print_array_uint8(uint8_t* arr, size_t size) {
    printf("[");
    for(unsigned i = 0; i < size; ++i) {
        if(i % 32 == 0 && (i != 0 && i != size-1))
            printf("%d,\n,", arr[i]);
        else
            printf("%d,", arr[i]);
    }
    printf("]\n");
}


static void receive_correct(client_t* const client, uint8_t* buf, const size_t init_num_faulty, const size_t batch_size, bool* corrects) {
    bool resend_rej = false;
    printf("faulty size: %lu\n", init_num_faulty);
    size_t num_faulty = init_num_faulty;
    uint8_t* faulty_queue = calloc(init_num_faulty, sizeof(uint8_t));
    uint8_t* ptr = faulty_queue;

    do {
        for(unsigned i = 0; i < batch_size; ++i) {
            if(!corrects[i]) {
                *ptr = i;
                ++ptr;
            }
        }
        // print_array_uint8(faulty_queue, init_num_faulty);
        send_REJ(client, num_faulty * sizeof(uint8_t), faulty_queue);
        size_t count_faulty = 0;
        for(unsigned i = 0; i < num_faulty; ++i) {
            com_t com;
            com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
            if (!com_receive(&com, true)) {
                resend_rej = true;
                ++count_faulty;
            }
            else {
                corrects[com.packet->nr] = true;
                if (client->quality <= 2)
                    decompress(&com);
                uint8_t* buf_ptr = buf + (com.packet->nr * constants_packets_size());
                memcpy(buf_ptr, com.packet->data, constants_packets_size());
            }
            free(com.packet->data);
            com_free(&com);
        }
        ptr = faulty_queue;
        num_faulty = count_faulty;
        printf("NEXT FAULTY BATCH SIZE: %lu\n", num_faulty);
    }
    while(resend_rej);
    free(faulty_queue);
}

void receive_batch(client_t* const client) {
    // Request batch
    send_RR(client);
    if (receive_EOS(client, false)) {
        client->EOS_received = true;
        return;
    }
    puts("============================");
    uint8_t quality = client->quality;
    size_t num_batch_packets = constants_batch_packets_amount(quality);
    size_t num_faulty = 0;
    bool package_correct[num_batch_packets];
    memset(package_correct, false, sizeof(bool) * num_batch_packets);
    // Receive batch and temporarily store in buf
    uint8_t* buf = malloc(num_batch_packets * constants_packets_size());
    for (unsigned i = 0; i < num_batch_packets; ++i) {
        com_t com;
        com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
        if (!com_receive(&com, true)) {
            ++num_faulty;
        }
        else {
            package_correct[com.packet->nr] = true;
            if (quality <= 2)
                decompress(&com);
            uint8_t* buf_ptr = buf + com.packet->nr * constants_packets_size();
            memcpy(buf_ptr, com.packet->data, com.packet->size);
            free(com.packet->data);
        }
        com_free(&com);
    }

    if(num_faulty > 0)
      receive_correct(client, buf, num_faulty, num_batch_packets, package_correct);
     puts("============================");
    // Place received data in player buffer
    for (unsigned i = 0; i < num_batch_packets; ++i) {
        uint8_t* buf_ptr = buf + i * constants_packets_size();
        buffer_add(client->player->buffer, buf_ptr, true);
    }
    free(buf);
    // batch received with success. Next time, ask next batch
    ++client->batch_nr;
}

bool receive_ACK(const client_t* const client, bool consume) {
    com_t com;
    printf("%p\n", (void*)client->sock);
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    bool ret = com_receive(&com, true);
    puts("receive_ack");
    bool is_ACK = flags_is_ACK(com.packet->flags);
    free(com.packet->data);
    com_free(&com);
    return ret && is_ACK;
}

bool receive_EOS(const client_t* const client, bool consume) {
    com_t com;
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    bool ret = com_receive(&com, false);
    printf("RECEIVE_EOS: %s\n", ret ? "TRUE" : "FALSE");
    bool is_EOS = flags_is_EOS(com.packet->flags);
    free(com.packet->data);
    com_free(&com);
    return ret && is_EOS;
}
