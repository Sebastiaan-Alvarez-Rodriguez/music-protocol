#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "client_info.h"

void client_info_init(client_info_t* const client, const com_t* const com, void* music_data) {
    memcpy(&client->client_addr, com->address, sizeof(struct sockaddr));
    client->stage = INITIAL;
    client->in_use = true;
    client->batch_ready = false;
    client->quality = malloc(sizeof(quality_t));
    client->music_ptr = music_data;
    client->bytes_sent = 0;
    client->packets_per_batch = 0;
    quality_init(client->quality, 0);
}

bool addr_in_cmp(const struct sockaddr_in* const c1, const struct sockaddr_in* const c2) {
    if (c1->sin_family != c2->sin_family)
        return false;

    bool port = (ntohs(c1->sin_port) == ntohs(c2->sin_port));
    bool ip = (ntohl(c1->sin_addr.s_addr) == ntohl(c2->sin_addr.s_addr));

    return port && ip;
}

uint8_t* get_music_chunk(client_info_t* const client, const unsigned packet_nr) {
    if(client->music_ptr == NULL)
        return NULL;

    uint8_t* ptr = client->music_ptr;
    ptr += (packet_nr * client->music_chuck_size);
    return ptr;
}

// TODO implement real calculation
size_t calculate_packet_size(const size_t buffer_size, const size_t batch_size) {
    return (buffer_size - (buffer_size % 16)) / batch_size;
}

void print_client_info(const client_info_t* const client) {
    if (client->in_use)
        printf("Client: {in_use : %u}, {ip : %s}, {port : %u}, {batch_nr : %u}, {current_q_level : %u}",
                client->in_use, 
                inet_ntoa(client->client_addr.sin_addr), 
                ntohs(client->client_addr.sin_port), 
                client->batch_ready, 
                client->quality->current);
    else
        printf("Client: {in_use : %u}, {ip : %s}, {port : %u}, {batch_nr : %u}, {current_q_level : -}",
                client->in_use, 
                inet_ntoa(client->client_addr.sin_addr), 
                ntohs(client->client_addr.sin_port), 
                client->batch_ready);
}

void client_info_free(client_info_t* const client) {
    free(client->quality);
}