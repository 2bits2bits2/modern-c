#ifndef HTTP_ERROR_H
#define HTTP_ERROR_H

#include "error.h"
#include "http.h"

/* Turn an app_error into a JSON HTTP response with the right status. */
void error_write_response(const struct app_error *err,
                          struct http_response *response);

#endif /* HTTP_ERROR_H */
