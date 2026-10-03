#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "http.h"

/*
 * A logging middleware wraps another handler. After the inner handler runs,
 * it reports the method, path and resulting status to a log function. Both
 * the next handler and the log function are injected, so the middleware is
 * testable with a spy.
 */
struct logged_handler {
    http_handler next;
    void *next_ctx;
    void (*log)(void *log_ctx, const char *method, const char *path,
                int status);
    void *log_ctx;
};

void logged_handle(const struct http_request *request,
                   struct http_response *response, void *ctx);

#endif /* MIDDLEWARE_H */
