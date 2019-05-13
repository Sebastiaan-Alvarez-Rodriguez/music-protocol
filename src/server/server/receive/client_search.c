#include <stdlib.h>
#include <stdio.h>

#include "client_search.h"

search_return search_client(server_t* const server, struct sockaddr* address, client_info_t** current) {
    client_info_t* ptr = server->clients;
    struct sockaddr_in* current_addr = (struct sockaddr_in*) address;
    unsigned cli_num = 0;
    for (unsigned i = 0; i < server->max_clients; ++i) {
        *current = &ptr[i];
        printf("INUSE =  %s\n", ptr[i].in_use ? "TRUE" : "FALSE");
        if (ptr[i].in_use && addr_in_cmp((struct sockaddr_in*) &ptr[i].client_addr, current_addr))
            return MATCH;
        else
            return MATCH_UNUSED;
    }
    *current = NULL;
    return NO_MATCH;
}
