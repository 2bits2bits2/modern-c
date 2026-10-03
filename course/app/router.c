#include "router.h"

#include <stdio.h>
#include <string.h>

static bool match_pattern(const char *pattern, const char *path, char *name,
                          size_t name_cap, char *value, size_t value_cap)
{
    const char *brace = strchr(pattern, '{');
    const char *close;
    const char *value_start;
    const char *suffix;
    size_t prefix_len;
    size_t suffix_len;
    size_t path_len;
    size_t value_len;
    size_t name_len;

    name[0] = '\0';
    value[0] = '\0';

    if (brace == NULL) {
        return strcmp(pattern, path) == 0;
    }

    close = strchr(brace, '}');
    if (close == NULL) {
        return false;
    }

    prefix_len = (size_t)(brace - pattern);
    if (strncmp(pattern, path, prefix_len) != 0) {
        return false;
    }

    value_start = path + prefix_len;
    suffix = close + 1;
    suffix_len = strlen(suffix);
    path_len = strlen(value_start);
    if (path_len < suffix_len) {
        return false;
    }
    if (strcmp(value_start + (path_len - suffix_len), suffix) != 0) {
        return false;
    }

    value_len = path_len - suffix_len;
    if (value_len == 0) {
        return false;
    }

    name_len = (size_t)(close - brace - 1);
    if (name_len >= name_cap) {
        name_len = name_cap - 1;
    }
    memcpy(name, brace + 1, name_len);
    name[name_len] = '\0';

    if (value_len >= value_cap) {
        value_len = value_cap - 1;
    }
    memcpy(value, value_start, value_len);
    value[value_len] = '\0';
    return true;
}

void router_dispatch(const struct router *router, struct http_request *request,
                     struct http_response *response)
{
    bool path_matched = false;

    for (size_t i = 0; i < router->count; i++) {
        const struct route *route = &router->routes[i];
        char name[32];
        char value[128];

        if (!match_pattern(route->pattern, request->path, name, sizeof name,
                           value, sizeof value)) {
            continue;
        }

        path_matched = true;

        if (strcmp(route->method, request->method) != 0) {
            continue;
        }

        if (name[0] != '\0') {
            snprintf(request->path_param, sizeof request->path_param, "%s",
                     value);
        } else {
            request->path_param[0] = '\0';
        }

        route->handler(request, response, router->ctx);
        return;
    }

    if (path_matched) {
        http_response_json(response, 405, "{\"error\":\"method not allowed\"}");
    } else {
        http_response_json(response, 404, "{\"error\":\"not found\"}");
    }
}
