#include "error.h"

#include <stdarg.h>
#include <stdio.h>

void error_clear(struct app_error *err)
{
    err->code = APP_OK;
    err->message[0] = '\0';
}

void error_set(struct app_error *err, int code, const char *fmt, ...)
{
    va_list args;

    err->code = code;

    va_start(args, fmt);
    vsnprintf(err->message, sizeof err->message, fmt, args);
    va_end(args);
}

bool error_is_set(const struct app_error *err)
{
    return err->code != APP_OK;
}

int error_http_status(int code)
{
    switch (code) {
    case APP_OK:
        return 200;
    case APP_ERR_INVALID:
        return 400;
    case APP_ERR_NOT_FOUND:
        return 404;
    case APP_ERR_CONFLICT:
        return 409;
    case APP_ERR_INTERNAL:
        return 500;
    default:
        return 500;
    }
}
