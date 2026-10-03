#ifndef HANDLER_H
#define HANDLER_H

#include "http.h"

/* The application's routes. ctx is a struct player_store *. */
void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx);

#endif /* HANDLER_H */
