#ifndef ASP
#define ASP
/*
 * Skeleton-code behorende bij het college Netwerken, opleiding Informatica,
 * Universiteit Leiden.
 */
#include <netinet/in.h>
#include <stdbool.h>

 /* An asp socket descriptor for information about the sockets current state */
 // struct asp_socket_info {
 //     int sockfd;
 //
 //     struct sockaddr_in local_addr;
 //     socklen_t local_addrlen;
 //
 //     struct sockaddr_in remote_addr;
 //     socklen_t remote_addrlen;
 //
 //     struct asp_socket_info *next;
 //
 //     int current_quality_level;
 //     int sequence_count;
 //
 //     int packets_received;
 //     int packets_missing;
 //
 //     struct timeval last_packet_received;
 //     struct timeval last_problem;
 //
 //     unsigned int is_connected : 1;
 //     unsigned int has_accepted : 1;
 // };

 /* An asp socket descriptor for information about the sockets current state */
 typedef struct {
    unsigned sockfd;

    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;

    unsigned current_q_level;
    unsigned sequence_count;

    unsigned packets_received;

    struct timeval last_packet_received;
    struct timeval last_problem;

    unsigned is_connected : 1;
    unsigned has_accepted : 1;
 } con_info_t;

void close_con(con_info_t* const conn);

#endif
