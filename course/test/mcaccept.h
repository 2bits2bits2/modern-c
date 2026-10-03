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

/* Like accept_spawn, but the child's stdout is connected to a pipe whose read
 * end is returned in *capture_fd. The caller closes it. */
pid_t accept_spawn_capture(const char *path, char *const argv[],
                           int *capture_fd);

/* Connect to an AF_UNIX stream socket at path, retrying until timeout_ms. */
int accept_connect_unix(const char *path, int timeout_ms);

/* Connect to host:port over TCP (IPv4), retrying until timeout_ms. */
int accept_connect_tcp(const char *host, int port, int timeout_ms);

/* Write request, then read the reply until the peer closes or cap is reached.
 * The reply is NUL-terminated. Returns the number of bytes read, or -1. */
ssize_t accept_roundtrip(int fd, const char *request, char *reply, size_t cap);

/* Read one line (up to '\n') from fd, NUL-terminated into buf. Returns the
 * length, or -1 on timeout or error. */
int accept_read_line(int fd, char *buf, size_t cap, int timeout_ms);

/* Read a line of the form "PORT=<n>" (or just "<n>") and return the number. */
int accept_read_port(int fd, int timeout_ms);

/* Send SIGTERM and wait up to timeout_ms for the child to exit. On success
 * *exit_status receives the waitpid status. If the child does not exit in
 * time it is killed, and the function returns false. */
bool accept_terminate(pid_t pid, int timeout_ms, int *exit_status);

/* Wait up to timeout_ms for the child to exit on its own. Does not signal. */
bool accept_wait(pid_t pid, int timeout_ms, int *exit_status);

#endif /* MCACCEPT_H */
