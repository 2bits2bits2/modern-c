#ifndef HANDLER_H
#define HANDLER_H

#include <stdbool.h>

#include "http.h"
#include "player_store.h"

/* Everything the request handlers need. */
struct app_context {
    struct player_store store;
    bool persist;
    char store_path[256];
};

void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx);

#endif /* HANDLER_H */
