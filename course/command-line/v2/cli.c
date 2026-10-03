#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "http_client.h"

int main(int argc, char **argv)
{
    int port = 0;
    int opt;
    const char *player;
    int wins;
    char reply[1024];
    int received;

    opterr = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        if (opt == 'p') {
            port = atoi(optarg);
        } else {
            fprintf(stderr, "usage: %s -p PORT PLAYER [WINS]\n", argv[0]);
            return 2;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "usage: %s -p PORT PLAYER [WINS]\n", argv[0]);
        return 2;
    }

    player = argv[optind++];
    wins = (optind < argc) ? atoi(argv[optind]) : 1;

    received =
        http_post_wins("127.0.0.1", port, player, wins, reply, sizeof reply);
    if (received < 0) {
        fprintf(stderr, "could not reach a server on port %d\n", port);
        return 1;
    }

    fputs(reply, stdout);
    return 0;
}
