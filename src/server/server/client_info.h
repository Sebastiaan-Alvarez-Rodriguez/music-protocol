#ifndef CLIENT_INFO
#define CLIENT_INFO

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "communication/com.h"

typedef struct {
   struct sockaddr_in client_addr;
   unsigned in_use : 1;
   unsigned batch_nr : 8;
   unsigned current_q_level : 4;
   struct timeval timeout_timer;
} client_info_t;

void client_info_init(client_info_t* client, const com_t* const com);

bool addr_in_cmp(const struct sockaddr_in* const c1, const struct sockaddr_in* const c2);

void print_client_info(const client_info_t* const client);
#endif
