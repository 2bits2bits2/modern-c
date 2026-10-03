#include "handler.h"

#include <stdio.h>
#include <string.h>

#include "json.h"
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

static void get_player(struct player_store *store, const char *name,
                       struct http_response *response)
{
    int wins = 0;
    char body[128];

    if (!player_store_get_wins(store, name, &wins)) {
        http_response_json(response, 404, "{\"error\":\"player not found\"}");
        return;
    }

    snprintf(body, sizeof body, "{\"name\":\"%s\",\"wins\":%d}", name, wins);
    http_response_json(response, 200, body);
}

static void get_league(struct player_store *store,
                       struct http_response *response)
{
    char body[2048];

    json_encode_league(store, body, sizeof body);
    http_response_json(response, 200, body);
}

static void post_player(struct player_store *store,
                        const struct http_request *request, const char *name,
                        struct http_response *response)
{
    long wins = 1;

    if (request->body_len > 0) {
        if (!json_get_int(request->body, "wins", &wins) || wins < 0) {
            http_response_json(response, 400,
                               "{\"error\":\"bad request body\"}");
            return;
        }
    }

    for (long i = 0; i < wins; i++) {
        player_store_record_win(store, name);
    }

    http_response_json(response, 201, "{\"status\":\"recorded\"}");
}

void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx)
{
    struct player_store *store = ctx;
    const char *name = player_name(request->path);

    if (strcmp(request->method, "GET") == 0) {
        if (name != NULL) {
            get_player(store, name, response);
        } else if (strcmp(request->path, "/league") == 0) {
            get_league(store, response);
        } else {
            http_response_json(response, 404, "{\"error\":\"not found\"}");
        }
        return;
    }

    if (strcmp(request->method, "POST") == 0) {
        if (name == NULL) {
            http_response_json(response, 404, "{\"error\":\"not found\"}");
            return;
        }
        post_player(store, request, name, response);
        return;
    }

    http_response_json(response, 405, "{\"error\":\"method not allowed\"}");
}
