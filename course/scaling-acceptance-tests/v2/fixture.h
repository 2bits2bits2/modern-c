#ifndef FIXTURE_H
#define FIXTURE_H

#include <stdbool.h>
#include <sys/types.h>

struct server_fixture {
    pid_t pid;
    char socket_path[128];
};

/* Start the server and wait until it is accepting connections. */
bool fixture_start(struct server_fixture *f, const char *server_path);

/* Connect to the running server. Caller closes the descriptor. */
int fixture_connect(struct server_fixture *f);

/* Ask the server to shut down and wait for it. */
bool fixture_stop(struct server_fixture *f);

#endif /* FIXTURE_H */
