#ifndef HANDLER_H
#define HANDLER_H

#include "http.h"

void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx);

#endif /* HANDLER_H */
