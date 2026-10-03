#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "handler.h"
#include "http.h"
#include "player_store.h"

static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static void seed(struct player_store *store, const char *spec)
{
    char name[PLAYER_NAME_MAX];
    const char *equals = strchr(spec, '=');
    int wins;

    if (equals == NULL) {
        return;
    }

    size_t len = (size_t)(equals - spec);
    if (len >= sizeof name) {
        len = sizeof name - 1;
    }
    memcpy(name, spec, len);
    name[len] = '\0';

    wins = atoi(equals + 1);
    for (int i = 0; i < wins; i++) {
        player_store_record_win(store, name);
    }
}

int main(int argc, char **argv)
{
    struct player_store store;
    int port = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }

    player_store_init(&store);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed(&store, argv[++i]);
        }
    }

    int listener = http_listen_tcp(port);
    if (listener < 0) {
        fprintf(stderr, "failed to listen on port %d\n", port);
        player_store_free(&store);
        return 1;
    }

    printf("PORT=%d\n", http_local_port(listener));
    fflush(stdout);

    struct sigaction action;
    memset(&action, 0, sizeof action);
    action.sa_handler = on_signal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    while (running) {
        int conn = accept(listener, NULL, NULL);

        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        http_serve_connection(conn, app_handler, &store);
        close(conn);
    }

    close(listener);
    player_store_free(&store);
    return 0;
}
