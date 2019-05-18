#include <netinet/in.h>
#include <stdbool.h>

#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "compression/compress.h"

#include "receive.h"

static void get_faulty(bool* corrects, const size_t corrects_size, uint8_t* const faulties) {
    uint8_t* ptr = faulties;
    for(unsigned i = 0; i < corrects_size; ++i) {
        if(!corrects[i]) {
            *ptr = i;
            ++ptr;
        }
    }
}

static bool receive_packet(const client_t* const client, size_t* const faulty_num, bool* corrects, uint8_t* buf) {
    com_t com;
    com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
    bool ret = false;
    if ((ret = !com_receive(&com, true))) {
        ++(*faulty_num);
    }
    else {
        corrects[com.packet->nr] = true;
        if (client->quality <= 2)
            decompress(&com);
        uint8_t* buf_ptr = buf + (com.packet->nr * constants_packets_size());
        memcpy(buf_ptr, com.packet->data, constants_packets_size());
        free(com.packet->data);
    }
    com_free(&com);
    return ret;
}


static void receive_correct(client_t* const client, uint8_t* buf, const size_t init_num_faulty, const size_t batch_size, bool* corrects) {
    bool resend_rej = false;

    uint8_t* faulty_queue = calloc(init_num_faulty, sizeof(uint8_t));
    size_t num_faulty = init_num_faulty;

    do {
        get_faulty(corrects, batch_size, faulty_queue);

        send_REJ(client, num_faulty * sizeof(uint8_t), faulty_queue);

        size_t count_faulty = 0;
        for(unsigned i = 0; i < num_faulty; ++i)
            receive_packet(client, &count_faulty, corrects, buf);

        num_faulty = count_faulty;
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
    size_t num_batch_packets = constants_batch_packets_amount(client->quality);
    size_t num_faulty = 0;

    bool package_correct[num_batch_packets];
    memset(package_correct, false, sizeof(bool) * num_batch_packets);

    // Receive batch and temporarily store in buf
    uint8_t* buf = malloc(num_batch_packets * constants_packets_size());

    for (unsigned i = 0; i < num_batch_packets; ++i)
        receive_packet(client, &num_faulty, package_correct, buf);

    if(num_faulty > 0)
        receive_correct(client, buf, num_faulty, num_batch_packets, package_correct);

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
    if(ret)
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
    if(ret)
        free(com.packet->data);
    com_free(&com);
    return ret && is_EOS;
}
