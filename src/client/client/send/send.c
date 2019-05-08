#include <arpa/inet.h>

#include "communication/com.h"
#include "communication/flags/flags.h"
#include "send.h"

void send_initial_communication(const client_t* const client) {
    com_t com;
    com_init(&com, client->fd, MSG_CONFIRM, client->sock, flags_get_raw(1, FLAG_ACK), 0);
    com_send(&com);
    com_free(&com);
}

void send_REJ(const client_t* const client, const size_t len, const uint16_t* package_nrs) {
    com_t com;
    com_init(&com, client->fd, MSG_CONFIRM, client->sock, flags_get_raw(1, FLAG_REJ), 0);
    com.packet->data = malloc(sizeof(uint32_t)+len);
    com.packet->size = sizeof(uint32_t)+len;
    memcpy(com.packet->data, &client->batch_nr, sizeof(uint32_t));
    
    uint32_t* data_ptr = com.packet->data;
    data_ptr++;
    memcpy(data_ptr, package_nrs, len);

    com_send(&com);

    free(com.packet->data);
    com_free(&com);
}

void send_RR(const client_t* const client) {
    com_t com;
    com_init(&com, client->fd, MSG_CONFIRM, client->sock, flags_get_raw(1, FLAG_RR), 0);
    com.packet->data = malloc(sizeof(uint32_t));

    memcpy(com.packet->data, &client->batch_nr, sizeof(uint32_t));
    com_send(&com);

    free(com.packet->data);
    com_free(&com);
}