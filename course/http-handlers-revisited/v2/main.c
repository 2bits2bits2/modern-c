#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "handler.h"
#include "http.h"
#include "middleware.h"
#include "persistence.h"

static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static void seed(struct app_context *app, const char *spec)
{
    char name[PLAYER_NAME_MAX];
    const char *equals = strchr(spec, '=');
    int wins;
    size_t len;

    if (equals == NULL) {
        return;
    }

    len = (size_t)(equals - spec);
    if (len >= sizeof name) {
        len = sizeof name - 1;
    }
    memcpy(name, spec, len);
    name[len] = '\0';

    wins = atoi(equals + 1);
    for (int i = 0; i < wins; i++) {
        player_store_record_win(&app->store, name);
    }
}

static void log_request(void *ctx, const char *method, const char *path,
                        int status)
{
    (void)ctx;
    fprintf(stderr, "%s %s -> %d\n", method, path, status);
}

int main(int argc, char **argv)
{
    struct app_context app;
    int port = 0;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--store") == 0 && i + 1 < argc) {
            snprintf(app.store_path, sizeof app.store_path, "%s", argv[++i]);
            app.persist = true;
        }
    }

    if (app.persist) {
        player_store_load(&app.store, app.store_path);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed(&app, argv[++i]);
        }
    }

    int listener = http_listen_tcp(port);
    if (listener < 0) {
        fprintf(stderr, "failed to listen on port %d\n", port);
        player_store_free(&app.store);
        return 1;
    }

    printf("PORT=%d\n", http_local_port(listener));
    fflush(stdout);

    struct sigaction action;
    memset(&action, 0, sizeof action);
    action.sa_handler = on_signal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    struct logged_handler logged = {
        .next = app_handler,
        .next_ctx = &app,
        .log = log_request,
        .log_ctx = NULL,
    };

    while (running) {
        int conn = accept(listener, NULL, NULL);

        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        http_serve_connection(conn, logged_handle, &logged);
        close(conn);
    }

    close(listener);
    player_store_free(&app.store);
    return 0;
}
