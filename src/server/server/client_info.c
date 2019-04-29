#include "client_info.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>

void client_info_init(client_info_t* client, const com_t* const com) {
    client->in_use = 1;
    memcpy(&client->client_addr, com->address, sizeof(struct sockaddr));
    client->batch_nr = 0;
    client->current_q_level = 5;
}

bool addr_in_cmp(const struct sockaddr_in* const c1, const struct sockaddr_in* const c2) {
    if (c1->sin_family != c2->sin_family)
        return false;

    bool port = (ntohs(c1->sin_port) == ntohs(c2->sin_port));
    bool ip = (ntohl(c1->sin_addr.s_addr) == ntohl(c2->sin_addr.s_addr));

    return port && ip;
}

void print_client_info(const client_info_t* const client) {
    printf("Client: {in_use : %u}, {ip : %s}, {port : %u}, {batch_nr : %u}, {current_q_level : %u}",
            client->in_use, inet_ntoa(client->client_addr.sin_addr), ntohs(client->client_addr.sin_port), client->batch_nr, client->current_q_level);
}
