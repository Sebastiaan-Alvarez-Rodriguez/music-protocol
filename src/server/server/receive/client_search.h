#ifndef CLIENT_SEARCH
#define CLIENT_SEARCH

#include <stdbool.h>

#include "server/server/server.h"

typedef enum {
    MATCH = 0,
    MATCH_UNUSED = 1,
    NO_MATCH = 2,
} search_return;

search_return search_client(server_t* const server, struct sockaddr* address, client_info_t** current);

#endif
