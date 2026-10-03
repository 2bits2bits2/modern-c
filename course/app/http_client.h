#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stddef.h>

/*
 * POST wins for name to a running server (our HTTP client, used by the CLI).
 * The raw response is copied into reply (NUL-terminated). Returns the number
 * of bytes received, or -1 if the server could not be reached.
 */
int http_post_wins(const char *host, int port, const char *name, int wins,
                   char *reply, size_t cap);

#endif /* HTTP_CLIENT_H */
