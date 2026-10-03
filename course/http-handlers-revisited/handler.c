#include "handler.h"

#include <stdio.h>
#include <string.h>

#include "json.h"
#include "persistence.h"
#include "router.h"

static void get_player(const struct http_request *request,
                       struct http_response *response, void *ctx)
{
    struct app_context *app = ctx;
    const char *name = request->path_param;
    struct player player;
    char body[256];
    int wins = 0;
    size_t len;

    if (!player_store_get_wins(&app->store, name, &wins)) {
        http_response_json(response, 404, "{\"error\":\"player not found\"}");
        return;
    }

    memset(&player, 0, sizeof player);
    len = strlen(name);
    if (len >= sizeof player.name) {
        len = sizeof player.name - 1;
    }
    memcpy(player.name, name, len);
    player.wins = wins;

    json_encode_player(&player, body, sizeof body);
    http_response_json(response, 200, body);
}

static void get_league(const struct http_request *request,
                       struct http_response *response, void *ctx)
{
    struct app_context *app = ctx;
    char body[2048];

    (void)request;
    player_store_sort_by_wins(&app->store);
    json_encode_league(&app->store, body, sizeof body);
    http_response_json(response, 200, body);
}

static void post_player(const struct http_request *request,
                        struct http_response *response, void *ctx)
{
    struct app_context *app = ctx;
    const char *name = request->path_param;
    long wins = 1;

    if (request->body_len > 0) {
        if (!json_get_int(request->body, "wins", &wins) || wins < 0) {
            http_response_json(response, 400,
                               "{\"error\":\"bad request body\"}");
            return;
        }
    }

    for (long i = 0; i < wins; i++) {
        player_store_record_win(&app->store, name);
    }

    if (app->persist) {
        player_store_save(&app->store, app->store_path);
    }

    http_response_json(response, 201, "{\"status\":\"recorded\"}");
}

void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx)
{
    static const struct route routes[] = {
        {"GET", "/players/{name}", get_player},
        {"POST", "/players/{name}", post_player},
        {"GET", "/league", get_league},
    };
    struct router router = {
        .routes = routes,
        .count = sizeof routes / sizeof routes[0],
        .ctx = ctx,
    };
    struct http_request copy = *request;

    /* router_dispatch records the path parameter on the request it is given. */
    router_dispatch(&router, &copy, response);
}
