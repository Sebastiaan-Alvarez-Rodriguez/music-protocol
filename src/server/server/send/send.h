#ifndef SEND
#define SEND

#include "server/server/server.h"
#include "server/server/task/task.h"
#include "server/client_info/client_info.h"
#include "communication/com.h"

bool send_to_client(server_t* const server, client_info_t* const current, const task_t* const task);

#endif
