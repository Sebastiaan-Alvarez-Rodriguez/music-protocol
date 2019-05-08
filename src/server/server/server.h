#ifndef SERVER
#define SERVER

#include <stdbool.h>
#include "server/music/music.h"
#include "asp.h"
#include "server/client_info/client_info.h"
#include "communication/com.h"

typedef struct {
    music_file* mf;
    unsigned short port;
    unsigned fd;
    unsigned max_clients;
    client_info_t* clients;
} server_t;

// Initialize a server struct
void server_init(server_t* const server);

// Set music file for server struct.
// Returns true on success, false otherwise
bool server_set_music(server_t* const server, const char* const musicfile);

// Set port for server struct. Returns true on success, false otherwise
bool server_set_port(server_t* const server, unsigned short port);

// Set max clients to connect to the server and allocate memory
// Returns true on successful allocation, false otherwise
bool server_set_num_clients(server_t* const server, const unsigned max_clients);

// Run a given server_t
void server_run(server_t* const server);

// Free server memory allocations
void server_free(server_t* const server);

// Prints the info on all clients
void print_clients(const server_t* const server);


#endif
