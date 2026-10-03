#ifndef ROUTER_H
#define ROUTER_H

#include <stddef.h>

#include "http.h"

struct route {
    const char *method;
    const char *pattern; /* exact, or with one {name} segment */
    http_handler handler;
};

struct router {
    const struct route *routes;
    size_t count;
    void *ctx; /* passed to every handler */
};

/*
 * Dispatch a request. Sets request->path_param for patterns with {name}.
 * Responds 405 if the path matches but the method does not, and 404 if
 * nothing matches.
 */
void router_dispatch(const struct router *router, struct http_request *request,
                     struct http_response *response);

#endif /* ROUTER_H */
