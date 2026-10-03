#ifndef APP_ERROR_H
#define APP_ERROR_H

#include <stdbool.h>

/*
 * A richer error than a bare code: a code to branch on and a message to show
 * or log. Callees fill it in; callers check error_is_set().
 */
struct app_error {
    int code;
    char message[128];
};

enum app_error_code {
    APP_OK = 0,
    APP_ERR_INVALID,
    APP_ERR_NOT_FOUND,
    APP_ERR_CONFLICT,
    APP_ERR_INTERNAL,
};

void error_clear(struct app_error *err);
void error_set(struct app_error *err, int code, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
bool error_is_set(const struct app_error *err);

/* Map a code onto an HTTP status. Unknown codes become 500. */
int error_http_status(int code);

#endif /* APP_ERROR_H */
