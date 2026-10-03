#ifndef MCACCEPT_H
#define MCACCEPT_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/*
 * mcaccept — helpers for acceptance tests that drive a real program as a
 * child process. Pair it with mctest, the way the chapters do.
 */

/* Build a unique AF_UNIX socket path from name and the current process id. */
void accept_socket_path(char *buf, size_t cap, const char *name);

/* Fork and exec path. argv is NULL-terminated and argv[0] is the program
 * name, as with execv. Returns the child pid, or -1. */
pid_t accept_spawn(const char *path, char *const argv[]);

/* Connect to an AF_UNIX stream socket at path, retrying until timeout_ms.
 * Returns a connected descriptor, or -1 if it never became ready. */
int accept_connect_unix(const char *path, int timeout_ms);

/* Write request, then read the reply until the peer closes or cap is reached.
 * The reply is NUL-terminated. Returns the number of bytes read, or -1. */
ssize_t accept_roundtrip(int fd, const char *request, char *reply, size_t cap);

/* Send SIGTERM and wait up to timeout_ms for the child to exit. On success
 * *exit_status receives the waitpid status. If the child does not exit in
 * time it is killed, and the function returns false. */
bool accept_terminate(pid_t pid, int timeout_ms, int *exit_status);

#endif /* MCACCEPT_H */
