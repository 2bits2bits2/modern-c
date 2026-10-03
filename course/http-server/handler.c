#include "handler.h"

#include <stdio.h>
#include <string.h>

#include "player_store.h"

static const char *player_name(const char *path)
{
    const char *prefix = "/players/";
    size_t prefix_len = strlen(prefix);

    if (strncmp(path, prefix, prefix_len) == 0 && path[prefix_len] != '\0') {
        return path + prefix_len;
    }
    return NULL;
}

void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx)
{
    struct player_store *store = ctx;

    if (strcmp(request->method, "GET") == 0) {
        const char *name = player_name(request->path);

        if (name != NULL) {
            int wins = 0;

            if (player_store_get_wins(store, name, &wins)) {
                char body[32];

                snprintf(body, sizeof body, "%d", wins);
                http_response_text(response, 200, body);
                return;
            }

            http_response_text(response, 404, "player not found");
            return;
        }
    }

    http_response_text(response, 404, "not found");
}
