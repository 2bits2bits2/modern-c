#include "http_error.h"

#include <stdio.h>

void error_write_response(const struct app_error *err,
                          struct http_response *response)
{
    char body[256];

    snprintf(body, sizeof body, "{\"error\":\"%s\"}", err->message);
    http_response_json(response, error_http_status(err->code), body);
}
