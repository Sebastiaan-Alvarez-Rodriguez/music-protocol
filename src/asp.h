#ifndef ASP
#define ASP
/*
 * Skeleton-code behorende bij het college Netwerken, opleiding Informatica,
 * Universiteit Leiden.
 */
 #include <netinet/in.h>


 /* An asp socket descriptor for information about the sockets current state */
 typedef struct {
     unsigned sockfd;

     struct sockaddr_in local_addr;
     struct sockaddr_in remote_addr;

     unsigned current_quality_level;
     unsigned sequence_count;

     unsigned packets_received;
     unsigned packets_missing;

     struct timeval last_packet_received;
     struct timeval last_problem;

     unsigned is_connected : 1;
     unsigned has_accepted : 1;
 } con_info_t;

void init_con(con_info_t* const conn, unsigned sockfd);


void close_con(con_info_t* const conn);

#endif
