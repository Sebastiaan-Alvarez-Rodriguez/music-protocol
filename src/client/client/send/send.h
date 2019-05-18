#ifndef SEND
#define SEND
#include "client/client/client.h"

// Send an ACK to the server
void send_initial_communication(client_t* const client);

// Send REJ a X to server, with 
// a = current batch number and X = all packetnumbers to resend
// len is the bytelength of the array
// package_nrs is a pointer to a buffer with all numbers to reject
// NOTE: A REJ data section is first 1 uint32_t containing batch_nr.
//       Then follow len - sizeof(uint32_t) bytes of uint8_t's
void send_REJ(const client_t* const client, const size_t len, const uint8_t* package_nrs);

// Send RR a to server, with a = current batch number
void send_RR(const client_t* const client);

// Send QTY flag with new quality to server
// The quality being sent is in client->quality->current
void send_QTY(client_t* const client);
#endif
