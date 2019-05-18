#ifndef RECEIVE
#define RECEIVE

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "communication/com.h"
#include "server/client_info/client_info.h"
#include "server/server/server.h"
#include "server/server/task/task.h"

// Receives a packet from a client and processes the request, setting
// the current pointer to the current connected client
bool receive_from_client(server_t* const server, com_t* receive, client_info_t** current, task_t* const task);


#endif
