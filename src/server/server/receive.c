#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>


#include "receive.h"
#include "communication/com.h"
#include "client_info.h"
#include "client_search.h"

// Receives a message from the client and registers the client if it is
// a new connection, otherwise points to the current connected client
static bool receive_and_register(server_t* const server, com_t* const receive, const client_info_t* current) {
    struct sockaddr_in client_address;
    com_init(receive, server->fd, MSG_WAITALL, (struct sockaddr*) &client_address, 0, 0);
    if (!receive_com(receive))
        return false;

    client_info_t* client = NULL;

    switch (search_client(server, receive->address, &client)) {
        case MATCH:
            puts("welcome back");
            break;
        case MATCH_UNUSED:
            puts("new user");
            printf("%p\n", client);
            client_info_init(client, receive);
            break;
        case NO_MATCH:
            puts("rejected, clients full");
            break;
        default:
            errno = EINVAL;
            return false;
    }
    current = client;
    return true;
}

bool initial_receive(server_t* const server) {
    com_t com;
    client_info_t* current = NULL;
    receive_and_register(server, &com, current);
    print_clients(server);

    return true;
}
