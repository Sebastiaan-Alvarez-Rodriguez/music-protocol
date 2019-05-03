#ifndef CLIENT
#define CLIENT
#include <stdint.h>

typedef struct {
    unsigned fd;
    struct sockaddr* sock;
    player_t* player;
} client_t;

// Initializes a client-struct by connecting to the server
void client_init(client_t* const client, const char* address, const unsigned short port);

bool send_initial_comunication(const client_t* const client, const uint16_t buffersize);

void client_free(client_t* const client);
#endif