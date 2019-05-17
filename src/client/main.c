#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "client/client/receive/receive.h"
#include "client/client/client.h"
#include "client/musicplayer/player.h"
#include "communication/constants/constants.h"

#include "buffer/buffer.h"

static void showHelp(const char *prog_name) {
    puts(prog_name);
    puts("[-q quality] [-i IP address] [-p port] [-d debug] [-h]");
    puts("");
    puts("-b buffersize Give a buffer size in KB");
    puts("-q quality    Number within [1-5] (higher means more quality)");
    puts("-i IP address IP address of server");
    puts("-p port       Specify which port the client should connect to");
    puts("-d debug      If set, prints debug information");
    puts("-h            Shows this dialog");
}

void run(const char* address, const unsigned short port, const unsigned buffer_size, const unsigned initial_quality) {
    client_t client;
    client_init(&client, address, port, buffer_size, initial_quality);
    client_fill_initial_buffer(&client);

    while(!client.EOS_received) {
        while (buffer_free_size(client.player->buffer) < constants_batch_packets_amount(client.quality))
            player_play(client.player);
        
        receive_batch(&client);
    }
    // Play the last buffered music
    while (!buffer_empty(client.player->buffer))
        player_play(client.player);

    client_free(&client);
}

int main(int argc, char **argv) {
    const char* const prog_name = argv[0];
    unsigned buffer_size = 1024;
    uint8_t initial_quality = 3;
    char* server_address = "127.0.0.1";
    unsigned short bind_port = 1235;

    char c;
    while ((c = getopt(argc, argv, "b:q:i:p:h")) != -1){
        switch (c) {
            case 'b':
                if (*optarg >= '1') {
                    buffer_size = atoi(optarg);
                    printf("Buffersize set to %u\n", buffer_size);
                    buffer_size *= 1024;
                } else {
                    puts("Provide a bufferspace >= 1");
                    return -1;
                }
                break;
            case 'q':
                if (*optarg >= '1' && *optarg <= '5') {
                    initial_quality = atoi(optarg);
                    printf("Quality set to %u\n", initial_quality);
                }
                else {
                    puts("Your quality level must be within [1-5]");
                    return -1;
                }
                break;
            case 'i':
                // TODO: gaat dit niet fout? (memory leaks?)
                server_address = optarg;
                break;
            case 'p':
                bind_port = (unsigned short) atoi(optarg);
                break;
            case 'h':
            default:
                showHelp(prog_name);
                return 0;
        }
    }
    argc -= optind;
    argv += optind;

    run(server_address, bind_port, buffer_size, initial_quality);
    return 0;
}
