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
#include "stats/stats.h"

typedef enum {
    INITIAL,
    INTERMEDIATE,
    FINAL
} stage_t;

typedef struct {
    struct sockaddr_in client_addr;

    stage_t stage;

    bool in_use;
    uint32_t batch_nr;
    quality_t* quality;
    stat_t* stat;

    int timeout_in_ms;

    size_t bytes_sent;
    size_t packets_per_batch;

    size_t buffer_size;
    size_t music_chuck_size;
    uint8_t* music_ptr;
} client_info_t;

// Initialize the client_info to the paramaters set in com_t
void client_info_init(client_info_t* const client, const com_t* const com, void* const music_data);

// Sets the timeout for a client in ms
void client_info_set_timeout(client_info_t* const client, const int timeout_ms);

// Compares two sockaddr_in structs, returns true if same address,
// else returns false
bool addr_in_cmp(const struct sockaddr_in* const c1, const struct sockaddr_in* const c2);

// Returns the portion of the music based on the given packet number
uint8_t* get_music_chunk(client_info_t* const client, const unsigned packet_nr);

// Prints a client_info struct
void print_client_info(const client_info_t* const client);

// Cleans up the client information
void client_info_free(client_info_t* const client);
#endif
