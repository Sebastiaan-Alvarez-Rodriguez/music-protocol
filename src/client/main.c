#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#include "client/client/receive/receive.h"
#include "client/client/client.h"
#include "client/musicplayer/player.h"
#include "communication/constants/constants.h"
#include "stats/stats.h"

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
        while (buffer_free_size(client.player->buffer) < constants_batch_packets_amount(client.quality->current))
            player_play(client.player);

        clock_t cur = clock();
        receive_batch(&client);
        client.stat->clock_diff += clock() - cur;
        client.stat->bytes += constants_batch_size(client.quality->current);

        client_print_stats(&client);
        client_adjust_quality(&client);
    }
    stat_print(client.stat);
    // Play the last buffered music
    while (!buffer_empty(client.player->buffer))
        player_play(client.player);

    client_free(&client);
}

int main(int argc, char **argv) {
    const char* const prog_name = argv[0];
    size_t buffer_size = constants_batch_size(5)*2;
    uint8_t initial_quality = 3;
    char* server_address = "127.0.0.1";
    unsigned short bind_port = 1235;

    char c;
    while ((c = getopt(argc, argv, "b:q:i:p:h")) != -1){
        switch (c) {
            case 'b':
                if (*optarg >= '1') {
                    size_t tmp_size = atoi(optarg);
                    if (tmp_size * 1024 >= buffer_size) {
                        printf("Buffersize set to %lu\n", tmp_size);
                        buffer_size = tmp_size * 1024;
                    } else {
                        puts("Buffer must at least contain one max batch.");
                        printf("At this point, the size of max batch is approx. %lu KB.\n", constants_batch_size(5)/1024 + 1);
                        puts("If this means that you cannot use this framework on your microwave, we are very sorry.");
                        return -1;
                    }
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
                    puts("Your quality level must be within [1-5].");
                    return -1;
                }
                break;
            case 'i':
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

    printf("Buffersize is set to approx. %lu KB.\n", buffer_size/1024);
    run(server_address, bind_port, buffer_size, initial_quality);
    return 0;
}
