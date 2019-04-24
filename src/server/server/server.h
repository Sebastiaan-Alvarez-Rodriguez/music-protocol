#ifndef SERVER
#define SERVER

#include <stdbool.h>
#include "server/music/music.h"

typedef struct {
    music_file* mf;
    unsigned short port;
    unsigned fd;
} server_t;

// Initialize a server struct
void server_init(server_t* const server);

// Set music file for server struct.
// Returns true on success, false otherwise
bool server_set_music(server_t* const server, const char* const musicfile);

// Set port for server struct. Returns true on success, false otherwise
bool server_set_port(server_t* const server, unsigned short port);

// Run a given server_t, with given initial quality
void server_run(server_t* const server, unsigned initial_quality);

// Free server memory allocations
void server_free(server_t* const server);


#endif