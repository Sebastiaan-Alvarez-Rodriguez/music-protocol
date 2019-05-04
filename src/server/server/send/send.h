#ifndef SEND
#define SEND

#include "server/server/server.h"
#include "server/client_info/client_info.h"


bool send_to_client(server_t* const server, client_info_t* const current);

#endif
