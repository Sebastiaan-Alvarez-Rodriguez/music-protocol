#ifndef RECEIVE
#define RECEIVE

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "communication/com.h"
#include "server/server/server.h"
#include "server/client_info/client_info.h"

// Receives a packet from a client and processes the request, setting
// the current pointer to the current connected client
bool receive_from_client(server_t* const server, com_t* const receive, client_info_t** current);


#endif
