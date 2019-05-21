#ifndef RECEIVE
#define RECEIVE
#include <time.h>
#include "client/client/client.h"

// Receive a batch from the server
void receive_batch(client_t* const client, clock_t* const start, size_t* bytes_received);

// Check if a ACK was received
// consume determines whether received packet(s) should be consumed
enum recv_flag receive_ACK(const client_t* const client, bool consume);

// Check if a EOS was received
// consume determines whether received packet(s) should be consumed
bool receive_EOS(const client_t* const client, bool consume);
#endif
