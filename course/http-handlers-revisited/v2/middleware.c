#include "middleware.h"

void logged_handle(const struct http_request *request,
                   struct http_response *response, void *ctx)
{
    struct logged_handler *logged = ctx;

    logged->next(request, response, logged->next_ctx);
    logged->log(logged->log_ctx, request->method, request->path,
                response->status);
}
