#include <netinet/in.h>

#include <errno.h>
#include "buffer/buffer.h"
#include "client/client/client.h"
#include "client/client/send/send.h"
#include "communication/constants/constants.h"
#include "communication/flags/flags.h"

#include "compression/compress.h"

#include "receive.h"

typedef struct {
    void* data_ptr;
    uint8_t* recv_nrs;
    uint8_t size_nrs;
} raw_batch_t;

static inline bool contains(raw_batch_t* raw, uint8_t item) {
    for (uint8_t i = 0; i < raw->size_nrs; ++i)
        if (((uint8_t*)raw->recv_nrs)[i] == item)
            return true;
    return false;
}

static inline uint8_t* raw_batch_get_missing_nrs(raw_batch_t* raw, uint8_t expected) {
    uint8_t* not_containing = malloc(sizeof(uint8_t)*(expected - raw->size_nrs));
    uint8_t* not_containing_ptr = not_containing;
    for (uint8_t i = 0; i < expected; ++i)
        if (!contains(raw, i)) {
            printf("Found missing: %u.\n", i);
            *not_containing_ptr = i;
            ++not_containing_ptr;
        }
    return not_containing;
}

static void raw_batch_init(raw_batch_t* raw, size_t expected_packet_amt) {
    raw->data_ptr = malloc(expected_packet_amt * constants_packets_size());
    raw->recv_nrs = malloc(expected_packet_amt);
    raw->size_nrs = 0;
}

static void raw_batch_free(raw_batch_t* raw) {
    free(raw->data_ptr);
    free(raw->recv_nrs);
}

static void raw_batch_receive(const client_t* const client, raw_batch_t* raw) {
    uint8_t initial_size_retrieved = raw->size_nrs;
    for (unsigned i = 0; i < (constants_batch_packets_amount(client->quality->current) - initial_size_retrieved); ++i) {
        com_t com;
        com_init(&com, client->fd, MSG_WAITALL, client->sock, FLAG_NONE, 0);
        enum recv_flag flag = com_receive(&com);
        if (flag == RECV_TIMEOUT) {
            client->quality->lost = (constants_batch_packets_amount(client->quality->current) - initial_size_retrieved) - i;
            break;
        } else if (flag == RECV_FAULTY) {
            client->quality->faulty += 1;
            continue;
        }
        if ((client->batch_nr == 1) && com.packet->nr == 42 && i != 0)
            continue;
        else if (com.packet->nr == 42 && i == 0) {
            printf("receiving packet 42.\nAmount left: %lu\n", 
                (constants_batch_packets_amount(client->quality->current) - initial_size_retrieved) -1);
            // for (unsigned i = 0; i < raw->size_nrs; ++i) {
            //     printf("Already have: %u\n", raw->recv_nrs[i]);
            // }
        }

        if (quality_suggest_compression(client->quality))
            decompress(&com);

        uint8_t* buf_ptr = (uint8_t*) raw->data_ptr + com.packet->nr * constants_packets_size();
        raw->recv_nrs[raw->size_nrs] = com.packet->nr;
        raw->size_nrs += 1;
        memcpy(buf_ptr, com.packet->data, com.packet->size);
        free(com.packet->data);
        com_free(&com);
    }
}

static inline bool raw_batch_integrity_ok(const client_t* const client, raw_batch_t* raw) {
    if (raw->size_nrs != constants_batch_packets_amount(client->quality->current)) {
        printf("I miss packets! QTY: %u. Expected: %lu. Got: %u.\n", 
            client->quality->current, 
            constants_batch_packets_amount(client->quality->current),
            raw->size_nrs);
        uint8_t* missing = raw_batch_get_missing_nrs(raw,constants_batch_packets_amount(client->quality->current));
        printf("Send REJ for %lu packets.\n", constants_batch_packets_amount(client->quality->current) - raw->size_nrs);
        send_REJ(client, constants_batch_packets_amount(client->quality->current) - raw->size_nrs, missing);
        free(missing);
        puts("Integrity failure");
        return false;
    }
    puts("Integrity ok");
    return true;
}

void receive_batch(client_t* const client) {
    // Request batch
    puts("New batch");
    send_RR(client);
    if (receive_EOS(client, false)) {
        client->EOS_received = true;
        return;
    }
    printf("Receiving %lu packets\n", constants_batch_packets_amount(client->quality->current));
    raw_batch_t raw;
    raw_batch_init(&raw, constants_batch_packets_amount(client->quality->current));
    do {
        raw_batch_receive(client, &raw);
    } while (!raw_batch_integrity_ok(client, &raw));
    client->quality->ok += constants_batch_packets_amount(client->quality->current);

    // Place received data in player buffer
    for (unsigned i = 0; i < constants_batch_packets_amount(client->quality->current); ++i) {
        uint8_t* buf_ptr = (uint8_t*) raw.data_ptr + i * constants_packets_size();
        buffer_add(client->player->buffer, buf_ptr, true);
    }
    raw_batch_free(&raw);
    // batch received with success. Next time, ask next batch
    ++client->batch_nr;
}

enum recv_flag receive_ACK(const client_t* const client, bool consume) {
    com_t com;
    printf("%p\n", (void*)client->sock);
    com_init(&com, client->fd, consume ? MSG_WAITALL : MSG_PEEK, client->sock, FLAG_NONE, 0);
    enum recv_flag flag = com_receive(&com);
    if (flag != RECV_OK) {
        printf("flag was not ok.\n");
        if (flag == RECV_TIMEOUT)
            printf("flag was TIMEOUT.\n");
        else if (flag == RECV_ERROR)
            printf("flag was RECV_ERROR.\n");
        return flag;
    }
    bool is_ACK = flags_is_ACK(com.packet->flags);
    free(com.packet->data);
    com_free(&com);
    return is_ACK ? RECV_OK : RECV_ERROR;
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
