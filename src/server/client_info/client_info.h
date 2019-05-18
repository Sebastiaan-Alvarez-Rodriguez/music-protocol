#ifndef CLIENT_INFO
#define CLIENT_INFO

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <stddef.h>
#include "communication/com.h"
#include "communication/quality/quality.h"

typedef enum {
    INITIAL,
    INTERMEDIATE,
    FINAL
} stage_t;

typedef struct {
   struct sockaddr_in client_addr;

   stage_t stage;

   bool in_use;
   bool batch_ready;
   quality_t* quality;
   struct timeval timeout_timer;

   uint32_t bytes_sent;
   size_t packets_per_batch;
   uint32_t batch_nr;

   size_t buffer_size;
   size_t music_chuck_size;
   uint8_t* music_ptr;
} client_info_t;

// Initialize the client_info to the paramaters set in com_t
void client_info_init(client_info_t* const client, const com_t* const com, void* const music_data);

// Compares two sockaddr_in structs, returns true if same address,
// else returns false
bool addr_in_cmp(const struct sockaddr_in* const c1, const struct sockaddr_in* const c2);

uint8_t* get_music_chunk(client_info_t* const client, const unsigned packet_nr);

size_t calculate_packet_size(const size_t buffer_size, const size_t batch_size);

// Prints a client_info struct
void print_client_info(const client_info_t* const client);

void client_info_free(client_info_t* const client);
#endif
