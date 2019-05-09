#ifndef CLIENT
#define CLIENT
#include <stdint.h>
#include "client/musicplayer/player.h"
#include "communication/com.h"
#include "communication/flags/flags.h"

typedef struct {
    unsigned fd;
    struct sockaddr* sock;
    player_t* player;
    uint8_t quality;

    unsigned batch_nr;
    bool EOS_received;
} client_t;

// Initializes a client_t by connecting to the server
void client_init(client_t* const client, const char* address, const unsigned short port, const unsigned buffer_size, const unsigned initial_quality);

// Fill the initial buffer as far as possible without fragmenting batches
void client_fill_initial_buffer(client_t* const client);

// Free a client_t
void client_free(client_t* const client);
#endif
