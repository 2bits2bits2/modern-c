#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"
#include "websocket.h"

static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static const char *header_value(const char *raw, const char *name)
{
    const char *p = strstr(raw, name);

    if (p == NULL) {
        return NULL;
    }
    p += strlen(name);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    return p;
}

static void handle(int conn)
{
    char raw[4096];
    ssize_t n = read(conn, raw, sizeof raw - 1);
    const char *key;
    char keybuf[128];
    char accept[64];
    char response[256];
    size_t k = 0;

    if (n <= 0) {
        return;
    }
    raw[n] = '\0';

    key = header_value(raw, "Sec-WebSocket-Key:");
    if (key == NULL) {
        const char *bad = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n"
                          "Connection: close\r\n\r\n";
        (void)write(conn, bad, strlen(bad));
        return;
    }

    while (key[k] != '\0' && key[k] != '\r' && key[k] != '\n' &&
           k + 1 < sizeof keybuf) {
        keybuf[k] = key[k];
        k++;
    }
    keybuf[k] = '\0';

    websocket_accept_key(keybuf, accept, sizeof accept);
    snprintf(response, sizeof response,
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n\r\n",
             accept);
    (void)write(conn, response, strlen(response));

    for (;;) {
        char payload[1024];
        int opcode = websocket_read_frame(conn, payload, sizeof payload);

        if (opcode < 0) {
            break;
        }
        if (opcode == 0x8) {
            websocket_write_close(conn);
            break;
        }
        if (opcode == 0x1) {
            websocket_write_text(conn, payload, strlen(payload));
        }
    }
}

int main(int argc, char **argv)
{
    int port = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }

    int listener = http_listen_tcp(port);
    if (listener < 0) {
        fprintf(stderr, "failed to listen on port %d\n", port);
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

        handle(conn);
        close(conn);
    }

    close(listener);
    return 0;
}
