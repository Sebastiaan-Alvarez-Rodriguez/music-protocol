#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "server/server/server.h"

static void show_help(const char *prog_name) {
    puts(prog_name);
    puts("[-q initial_quality] [-f soundfile] [-p port] [-d debug] [-h]");
    puts("");
    puts("-q quality    Specify initial initial_quality. Number within [1-5]");
    puts("-f soundfile  The soundfile to send to the client");
    puts("-p port       Specify which port the client should connect to");
    puts("-h            Shows this dialog");
}

int main(int argc, char** argv) {
    unsigned short port = 1235;
    char c;
    char* filename = NULL;
    unsigned max_clients = 5;
    const char* const prog_name = argv[0];
    while ((c = getopt(argc, argv, "f:p:h")) != -1){
        switch (c) {
            case 'f':
                if (optarg != NULL)
                    filename = optarg;
                break;
            case 'p':
                port = (unsigned short) atoi(optarg);
                break;
            case 'h':
            default:
                show_help(prog_name);
                return 0;
        }
    }
    argc -= optind;
    argv += optind;

    srand((unsigned) time(NULL));

    if (!filename) {
        puts("Please specify a sound file to open with -f <name>");
        return -1;
    }

    server_t server;
    server_init(&server);

    if (!server_set_music(&server, filename))
        return -1;
    if (!server_set_port(&server, port))
        return -1;
    if (!server_set_num_clients(&server, max_clients))
        return -1;

    server_run(&server);

    server_free(&server);
    return 0;
}
